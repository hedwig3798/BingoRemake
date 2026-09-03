#include "LoginProcessor.h"
#include "Server.h"
#include "Password.h"

uint32_t LoginProcessor::m_sessionCount = 0;

LoginProcessor::LoginProcessor(LuaHelper* _luaHelper)
	: m_luaHelper(_luaHelper)
	, m_reader()
	, m_gameServerIP()
	, m_gameServerPort()
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
		case CTL_RES_LOGIN::PACKET_ID:
		{
			CTL_RES_LOGIN dData;
			m_reader.SetBuffer(data, sizeof(PacketHeader));
			Deserialize(m_reader, dData);
			_CTL_RES_LOGIN(session, std::move(dData));
			break;
		}
		case CTL_RES_ID_AVAILABLITY::PACKET_ID:
		{
			CTL_RES_ID_AVAILABLITY dData;
			m_reader.SetBuffer(data, sizeof(PacketHeader));
			Deserialize(m_reader, dData);
			_CTL_RES_ID_AVAILABLITY(session, std::move(dData));
			break;
		}
		case DTA_ACK_ACCESS::PACKET_ID:
		{
			std::cout << "데이터 베이스 접속 성공\n";
			break;
		}
		case DTL_ACK_LOGIN_DATA::PACKET_ID:
		{
			DTL_ACK_LOGIN_DATA dData;
			m_reader.SetBuffer(data, sizeof(PacketHeader));
			Deserialize(m_reader, dData);
			_DTL_ACK_LOGIN_DATA(std::move(dData));
			break;
		}
		case DTL_ACK_ID_AVAILABLITY::PACKET_ID:
		{
			DTL_ACK_ID_AVAILABLITY dData;
			m_reader.SetBuffer(data, sizeof(PacketHeader));
			Deserialize(m_reader, dData);
			_DTL_ACK_ID_AVAILABLITY(std::move(dData));
			break;
		}
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
	data.m_sessionCount = m_sessionCount++;

	std::cout << data.m_ID << " " << data.m_hashedPW << std::endl;

	m_sessionMap[data.m_sessionCount] = _session;

	m_dbSession->SendPacket<LTD_RES_LOGIN_DATA>(data);

	return;
}

void LoginProcessor::_CTL_RES_ID_AVAILABLITY(std::shared_ptr<Session> _session, CTL_RES_ID_AVAILABLITY&& _data)
{
	std::cout << "_CTL_RES_ID_AVAILABLITY\n";
	LTD_RES_ID_AVAILABLITY result;
	result.m_ID = _data.m_ID;
	result.m_sessionCount = m_sessionCount++;
	m_sessionMap[result.m_sessionCount] = _session;

	m_dbSession->SendPacket<LTD_RES_ID_AVAILABLITY>(result);

	return;
}

void LoginProcessor::_DTL_ACK_LOGIN_DATA(DTL_ACK_LOGIN_DATA&& _data)
{
	LTC_ACK_LOGIN result;

	auto itr = m_sessionMap.find(_data.m_sessionCount);
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
	std::cout << "_DTL_ACK_ID_AVAILABLITY\n";

	LTC_ACK_ID_AVAILABLITY result;

	auto itr = m_sessionMap.find(_data.m_sessionCount);
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
