#include "LoginProcessor.h"

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
		case LOGIN_PACKET_SEND::PACKET_ID:
		{
			LOGIN_PACKET_SEND dData;
			m_reader.SetBuffer(data, sizeof(PacketHeader));
			Deserialize(m_reader, dData);
			_LOGIN_PACKET_SEND(session, std::move(dData));
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
	std::lock_guard<std::mutex> lock(m_qLock);
	m_msgQ.push({ _session, std::move(_buffer) });
}

void LoginProcessor::Init()
{
	m_gameServerIP = m_luaHelper->Get<std::string>("GameServerIP");
	m_gameServerPort = m_luaHelper->Get<short>("GameServerPort");
}

void LoginProcessor::_LOGIN_PACKET_SEND(std::shared_ptr<Session> _session, LOGIN_PACKET_SEND&& _data)
{
	std::cout << _data.m_ID << std::endl;
	std::cout << _data.m_hashedPW << std::endl;


	return;
}
