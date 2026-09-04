#include "DBProcessor.h"

DBProcessor::DBProcessor(LuaHelper* _luaHelper)
	: m_luaHelper(_luaHelper)
	, m_reader()
	, m_DBConnectionCount(0)
	, m_DBURL()
	, m_loginSession(nullptr)
	, m_gameSession(nullptr)
	, m_dbIoContext()
	, m_workGuard(boost::asio::make_work_guard(m_dbIoContext))
{

}

DBProcessor::~DBProcessor()
{
}

bool DBProcessor::Process()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(m_qLock);
		m_qcv.wait_for(
			lock
			, std::chrono::seconds(1)
			, [this]()
			{
				return !m_msgQ.empty();
			}
		);

		if (true == m_msgQ.empty())
		{
			continue;
		}

		std::shared_ptr<Session> session = m_msgQ.front().first;
		std::vector<char> data = std::move(m_msgQ.front().second);
		m_msgQ.pop();
		lock.unlock();

		PacketHeader* header = reinterpret_cast<PacketHeader*>(data.data());

		switch (header->m_ID)
		{
		case LTD_RES_ACCESS::PACKET_ID:
		{
			m_loginSession = session;
			m_loginSession->SendPacket<DTA_ACK_ACCESS>({});
			break;
		}
		case GTD_RES_ACCESS::PACKET_ID:
		{
			m_gameSession = session;
			break;
		}

		DESERIALIZE_RES_PACKET(LTD_RES_LOGIN_DATA, session, data);
		DESERIALIZE_RES_PACKET(LTD_RES_ID_AVAILABLITY, session, data);
		DESERIALIZE_RES_PACKET(LTD_RES_CREATE_USER_DATA, session, data);

		default:
			break;
		}
	}

	return true;
}

void DBProcessor::AddMsg(std::shared_ptr<Session> _session, std::vector<char>&& _buffer)
{
	{
		std::lock_guard<std::mutex> lock(m_qLock);
		m_msgQ.push({ _session, std::move(_buffer) });
	}

	m_qcv.notify_one();
}

void DBProcessor::Init()
{
	m_DBConnectionCount = m_luaHelper->Get<int>("DBConnectionCount");
	m_DBURL = m_luaHelper->Get<std::string>("DBURL");

	for (int i = 0; i < m_DBConnectionCount; ++i)
	{
		PGconn* conn = PQconnectdb(m_DBURL.c_str());
		if (CONNECTION_OK != PQstatus(conn))
		{
			std::cerr << "Supabase 연결 실패: " << PQerrorMessage(conn) << std::endl;
			PQfinish(conn);
			return;
		}

		int nativeSocket = PQsocket(conn);

		if (0 > nativeSocket)
		{
			std::cerr << "Supabase 소켓 획득 실패" << std::endl;
			PQfinish(conn);
			return;
		}

		DBConnection* connection = new DBConnection({ conn, boost::asio::ip::tcp::socket(m_dbIoContext) });
		boost::system::error_code ec;
		connection->m_socket.assign(boost::asio::ip::tcp::v4(), nativeSocket, ec);

		if (ec)
		{
			std::cerr << "소켓 할당 실패 : " << ec.message() << std::endl;
		}

		m_connQ.push(connection);
	}

	m_dbIoThread = std::thread([this]()
		{
			m_dbIoContext.run();
		}
	);

	std::cout << "데이터베이스 연결 성공\n";
}

void DBProcessor::ConnectServer(std::shared_ptr<Server> _server)
{

}

void DBProcessor::ReturnConnection(DBConnection* conn)
{
	std::lock_guard<std::mutex> lock(m_connQMutex);
	m_connQ.push(conn);
	m_connQCV.notify_one();
}

DBConnection* DBProcessor::GetConnection()
{
	std::unique_lock<std::mutex> lock(m_connQMutex);
	m_connQCV.wait(lock, [this]() { return false == m_connQ.empty(); });

	DBConnection* conn = m_connQ.front();
	if (PQTRANS_IDLE != PQtransactionStatus(conn->m_apiConnection))
	{
		// 예외처리
	}

	m_connQ.pop();
	lock.unlock();

	return conn;
}

void DBProcessor::_LTD_RES_LOGIN_DATA(std::shared_ptr<Session> _session, LTD_RES_LOGIN_DATA&& _data)
{
	DBConnection* conn = GetConnection();

	PGconn* apiConnection = conn->m_apiConnection;
	auto& socket = conn->m_socket;

	// 여기서 DB에 쿼리
	const char* paramValues[1] = { _data.m_ID.c_str() };
	int sendResult = PQsendQueryParams
	(
		apiConnection
		, "SELECT * FROM LoginFunction($1)"
		, 1
		, nullptr
		, paramValues
		, nullptr
		, nullptr
		, 0
	);

	if (sendResult == 0)
	{
		std::cerr << "Send failed: " << PQerrorMessage(apiConnection) << std::endl;
		ReturnConnection(conn);
		return;
	}

	_DTL_ACK_LOGIN_DATA(_session, conn, _data.m_ID, _data.m_hashedPW, _data.m_requestID);
}

void DBProcessor::_DTL_ACK_LOGIN_DATA(std::shared_ptr<Session> _session, DBConnection* _conn, std::string _id, std::string _hashedPW, uint32_t _sesstionCount)
{
	PGconn* apiConnection = _conn->m_apiConnection;
	auto& socket = _conn->m_socket;

	socket.async_wait(
		boost::asio::socket_base::wait_read
		, [this, _conn, _session, _id, _hashedPW, _sesstionCount](const boost::system::error_code& ec)
		{
			if (ec)
			{
				std::cerr << "DB 메세지 수신 실패 : " << ec.message() << std::endl;
				DTL_ACK_LOGIN_DATA data;
				data.m_ID = _id;
				data.m_requestID = _sesstionCount;
				data.m_netError = NET_ERROR::UNKNOWN_ERROR;
				m_loginSession->SendPacket(data);
				return;
			}

			if (PQconsumeInput(_conn->m_apiConnection) == 0)
			{
				ReturnConnection(_conn);
				DTL_ACK_LOGIN_DATA data;
				data.m_ID = _id;
				data.m_requestID = _sesstionCount;
				data.m_netError = NET_ERROR::UNKNOWN_ERROR;
				m_loginSession->SendPacket(data);
				return;
			}

			if (PQisBusy(_conn->m_apiConnection))
			{
				_DTL_ACK_LOGIN_DATA(_session, _conn, _id, _hashedPW, _sesstionCount);
				return;
			}

			PGresult* res = nullptr;
			while ((res = PQgetResult(_conn->m_apiConnection)) != nullptr)
			{
				int rowCount = PQntuples(res);
				if (0 == rowCount)
				{
					DTL_ACK_LOGIN_DATA data;
					data.m_ID = _id;
					data.m_requestID = _sesstionCount;
					data.m_netError = NET_ERROR::NO_ID_EXITS;
					m_loginSession->SendPacket(data);
				}
				else
				{
					DTL_ACK_LOGIN_DATA data;
					data.m_ID = _id;
					data.m_hashedPW = _hashedPW;
					data.m_saltedPW = PQgetvalue(res, 0, 0);
					data.m_salt = PQgetvalue(res, 0, 1);
					data.m_requestID = _sesstionCount;
					data.m_netError = NET_ERROR::NET_OK;
					m_loginSession->SendPacket(data);
				}
				PQclear(res);
			}

			ReturnConnection(_conn);
		}
	);
}

void DBProcessor::_LTD_RES_ID_AVAILABLITY(std::shared_ptr<Session> _session, LTD_RES_ID_AVAILABLITY&& _data)
{

	DBConnection* conn = GetConnection();
	PGconn* apiConnection = conn->m_apiConnection;
	auto& socket = conn->m_socket;

	const char* paramValues[1] = { _data.m_ID.c_str() };
	int sendResult = PQsendQueryParams
	(
		apiConnection
		, "SELECT * FROM availability_check($1)"
		, 1
		, nullptr
		, paramValues
		, nullptr
		, nullptr
		, 0
	);

	if (0 == sendResult)
	{
		DTL_ACK_ID_AVAILABLITY result;
		result.m_netError = NET_ERROR::DB_SEND_ERROR;
		result.m_requestID = _data.m_requestID;
		_session->SendPacket(result);
		return;
	}

	_DTL_ACK_ID_AVAILABLITY(_session, conn, _data.m_requestID);
}

void DBProcessor::_DTL_ACK_ID_AVAILABLITY(std::shared_ptr<Session> _session, DBConnection* _conn, uint32_t _sesstionCount)
{

	PGconn* apiConnection = _conn->m_apiConnection;
	auto& socket = _conn->m_socket;

	socket.async_wait(
		boost::asio::socket_base::wait_read
		, [this, _conn, _session, _sesstionCount](const boost::system::error_code& ec)
		{
			if (ec)
			{
				std::cerr << "DB 메세지 수신 실패 : " << ec.message() << std::endl;
				DTL_ACK_ID_AVAILABLITY data;
				data.m_isExist = false;
				data.m_requestID = _sesstionCount;
				data.m_netError = NET_ERROR::UNKNOWN_ERROR;
				m_loginSession->SendPacket(data);
				return;
			}

			if (PQconsumeInput(_conn->m_apiConnection) == 0)
			{
				ReturnConnection(_conn);
				DTL_ACK_ID_AVAILABLITY data;
				data.m_isExist = false;
				data.m_requestID = _sesstionCount;
				data.m_netError = NET_ERROR::UNKNOWN_ERROR;
				m_loginSession->SendPacket(data);
				return;
			}

			if (PQisBusy(_conn->m_apiConnection))
			{
				_DTL_ACK_ID_AVAILABLITY(_session, _conn, _sesstionCount);
				return;
			}

			PGresult* res = nullptr;
			while ((res = PQgetResult(_conn->m_apiConnection)) != nullptr)
			{
				DTL_ACK_ID_AVAILABLITY data;
				char* val = PQgetvalue(res, 0, 0);
				if (nullptr == val)
				{
					data.m_requestID = _sesstionCount;
					data.m_isExist = false;
					data.m_netError = NET_ERROR::UNKNOWN_ERROR;
					m_loginSession->SendPacket(data);
					return;
				}

				bool isExist = ('t' == val[0]);

				data.m_isExist = isExist;
				data.m_requestID = _sesstionCount;
				data.m_netError = NET_ERROR::NET_OK;
				m_loginSession->SendPacket(data);

				PQclear(res);
			}

			ReturnConnection(_conn);
		}
	);
}

void DBProcessor::_LTD_RES_CREATE_USER_DATA(std::shared_ptr<Session> _session, LTD_RES_CREATE_USER_DATA&& _data)
{
	DBConnection* conn = GetConnection();
	PGconn* apiConnection = conn->m_apiConnection;
	auto& socket = conn->m_socket;

	const char* paramValues[3] = { _data.m_ID.c_str(), _data.m_saltedPW.c_str(), _data.m_salt.c_str() };
	int sendResult = PQsendQueryParams
	(
		apiConnection
		, "SELECT * FROM AddNewAccount($1, $2, $3)"
		, 3
		, nullptr
		, paramValues
		, nullptr
		, nullptr
		, 0
	);

	if (0 == sendResult)
	{
		DTL_ACK_ID_AVAILABLITY result;
		result.m_netError = NET_ERROR::DB_SEND_ERROR;
		result.m_requestID = _data.m_requestID;
		_session->SendPacket(result);
		return;
	}

	_DTL_ACK_CREATE_USER_DATA(_session, conn, _data.m_requestID);
}

void DBProcessor::_DTL_ACK_CREATE_USER_DATA(std::shared_ptr<Session> _session, DBConnection* _conn, uint32_t _requestID)
{
	PGconn* apiConnection = _conn->m_apiConnection;
	auto& socket = _conn->m_socket;

	socket.async_wait(
		boost::asio::socket_base::wait_read
		, [this, _conn, _session, _requestID](const boost::system::error_code& ec)
		{
			if (ec)
			{
				std::cerr << "DB 메세지 수신 실패 : " << ec.message() << std::endl;
				DTL_ACK_ID_AVAILABLITY data;
				data.m_isExist = false;
				data.m_requestID = _requestID;
				data.m_netError = NET_ERROR::UNKNOWN_ERROR;
				m_loginSession->SendPacket(data);
				return;
			}

			if (PQconsumeInput(_conn->m_apiConnection) == 0)
			{
				ReturnConnection(_conn);
				DTL_ACK_CREATE_USER_DATA data;
				data.m_netError = NET_ERROR::UNKNOWN_ERROR;
				m_loginSession->SendPacket(data);
				return;
			}

			if (PQisBusy(_conn->m_apiConnection))
			{
				_DTL_ACK_CREATE_USER_DATA(_session, _conn, _requestID);
				return;
			}

			PGresult* res = nullptr;
			while ((res = PQgetResult(_conn->m_apiConnection)) != nullptr)
			{
				DTL_ACK_CREATE_USER_DATA data;
				char* val = PQgetvalue(res, 0, 0);
				if (nullptr == val)
				{
					data.m_requestID = _requestID;
					data.m_netError = NET_ERROR::DB_QUERY_ERROR;
					m_loginSession->SendPacket(data);
					return;
				}

				bool isSuccess = ('t' == val[0]);

				if (true == isSuccess)
				{
					data.m_requestID = _requestID;
					data.m_netError = NET_ERROR::NET_OK;
					m_loginSession->SendPacket(data);
					return;
				}

				data.m_requestID = _requestID;
				data.m_netError = NET_ERROR::ID_EXITS;
				m_loginSession->SendPacket(data);

				PQclear(res);
			}

			ReturnConnection(_conn);
		}
	);
}
