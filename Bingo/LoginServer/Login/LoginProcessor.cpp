#include "LoginProcessor.h"
#include "Server.h"
#include "Password.h"
#include "stringUtil.h"

uint32_t LoginProcessor::m_requestID = 0;

LoginProcessor::LoginProcessor(LuaHelper* _luaHelper)
	: m_luaHelper(_luaHelper)
	, m_reader()
	, m_gameServerIP()
	, m_gameServerPort()
	, m_DBServerIP()
	, m_DBServerPort()
{
}

LoginProcessor::~LoginProcessor()
{
}

bool LoginProcessor::Process()
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

			/// 클라이언트 패킷 처리 구문
			DESERIALIZE_RES_PACKET(CTL_RES_LOGIN, session, data);
			DESERIALIZE_RES_PACKET(CTL_RES_ID_AVAILABLITY, session, data);
			DESERIALIZE_RES_PACKET(CTL_RES_SING_UP, session, data);

			/// 이 이하로 DB 패킷 처리
			DESERIALIZE_ACK_PACKET(DTL_ACK_LOGIN_DATA, data);
			DESERIALIZE_ACK_PACKET(DTL_ACK_ID_AVAILABLITY, data);
			DESERIALIZE_ACK_PACKET(DTL_ACK_CREATE_USER_DATA, data);
		default:
			break;
		}
	}

	return true;
}

void LoginProcessor::AddMsg(std::shared_ptr<Session> _session, std::vector<char>&& _buffer)
{
	{
		std::lock_guard<std::mutex> lock(m_qLock);
		m_msgQ.push({ _session, std::move(_buffer) });
	}

	m_qcv.notify_one();
}

void LoginProcessor::Init()
{
	m_gameServerIP = m_luaHelper->Get<std::string>("GameServerIP");
	m_gameServerPort = m_luaHelper->Get<short>("GameServerPort");

	m_DBServerIP = m_luaHelper->Get<std::string>("DBServerIP");
	m_DBServerPort = m_luaHelper->Get<short>("DBServerPort");
}

void LoginProcessor::ConnectServer(std::shared_ptr<Server> _server)
{
	std::cout << "데이터 베이스 접속 시도\n";

	m_dbSession = _server->ConnectServer(m_DBServerIP, m_DBServerPort);
	m_dbSession->SendPacket<LTD_RES_ACCESS>({});
}

std::string LoginProcessor::GetSaltedString(const std::string& _string, const std::string& _salt)
{
	std::string result = _string + _salt;
	result = PW::HashSHA256S(result);

	return result;
}

void LoginProcessor::_CTL_RES_LOGIN(std::shared_ptr<Session> _session, CTL_RES_LOGIN&& _data)
{
	LTD_RES_LOGIN_DATA data;
	data.m_ID = _data.m_ID;
	data.m_hashedPW = _data.m_hashedPW;
	data.m_netError = NET_ERROR::NET_OK;
	data.m_requestID = m_requestID++;

	std::cout << data.m_ID << " " << data.m_hashedPW << std::endl;

	m_sessionMap[data.m_requestID] = _session;

	m_dbSession->SendPacket<LTD_RES_LOGIN_DATA>(data);

	return;
}

void LoginProcessor::_CTL_RES_ID_AVAILABLITY(std::shared_ptr<Session> _session, CTL_RES_ID_AVAILABLITY&& _data)
{
	LTD_RES_ID_AVAILABLITY result;
	result.m_ID = _data.m_ID;
	result.m_requestID = m_requestID++;
	m_sessionMap[result.m_requestID] = _session;

	m_dbSession->SendPacket<LTD_RES_ID_AVAILABLITY>(result);

	return;
}

void LoginProcessor::_CTL_RES_SING_UP(std::shared_ptr<Session> _session, CTL_RES_SING_UP&& _data)
{
	LTD_RES_CREATE_USER_DATA result;
	result.m_ID = _data.m_ID;
	result.m_salt = STRING_UTILS::GenerateRandomString(64);
	result.m_saltedPW = PW::HashSHA256S(_data.m_hashedPW + result.m_salt);
	result.m_requestID = m_requestID++;
	m_sessionMap[result.m_requestID] = _session;

	m_dbSession->SendPacket(result);
}

void LoginProcessor::_DTL_ACK_LOGIN_DATA(DTL_ACK_LOGIN_DATA&& _data)
{
	LTC_ACK_LOGIN result;

	auto itr = m_sessionMap.find(_data.m_requestID);
	if (itr == m_sessionMap.end())
	{
		std::cerr << "cannot find user connection\n";
		return;
	}

	switch (_data.m_netError)
	{
		// 비밀 번호 확인
	case NET_ERROR::NET_OK:
	{
		std::string salted = GetSaltedString(_data.m_hashedPW, _data.m_salt);

		if (salted != _data.m_saltedPW)
		{
			result.m_isSuccess = false;
			result.m_netError = NET_ERROR::PW_NOT_MATCH;
			itr->second->SendPacket(result);
			return;
		}

		result.m_isSuccess = true;
		result.m_netError = NET_ERROR::NET_OK;
		itr->second->SendPacket(result);

		// 세션 연결끊기

		break;
	}
	default:
	{
		result.m_isSuccess = false;
		result.m_netError = _data.m_netError;
		itr->second->SendPacket(result);

		break;
	}
	}
}

void LoginProcessor::_DTL_ACK_ID_AVAILABLITY(DTL_ACK_ID_AVAILABLITY&& _data)
{
	LTC_ACK_ID_AVAILABLITY result;

	auto itr = m_sessionMap.find(_data.m_requestID);
	if (itr == m_sessionMap.end())
	{
		std::cerr << "cannot find user connection\n";
		return;
	}

	switch (_data.m_netError)
	{
	case NET_ERROR::NET_OK:
	{
		result.m_netError = _data.m_netError;
		result.m_isExist = _data.m_isExist;
		itr->second->SendPacket(result);
		break;
	}
	default:
	{
		result.m_netError = _data.m_netError;
		result.m_isExist = false;
		itr->second->SendPacket(result);
		break;
	}
	}
}

void LoginProcessor::_DTL_ACK_CREATE_USER_DATA(DTL_ACK_CREATE_USER_DATA&& _data)
{
	LTC_ACK_SING_UP result;

	auto itr = m_sessionMap.find(_data.m_requestID);
	if (itr == m_sessionMap.end())
	{
		std::cerr << "cannot find user connection\n";
		return;
	}

	result.m_netError = _data.m_netError;
	itr->second->SendPacket(result);
}
