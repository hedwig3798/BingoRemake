#pragma once

#include "IProcessor.h"
#include "Packet.h"
#include "Serializer.h"
#include "luaHelper.h"
#include "Session.h"
#include <memory>
#include <queue>
#include <mutex>
#include <unordered_map>

class LoginProcessor
	: public IProcessor
{
private:
	std::queue<std::pair<std::shared_ptr<Session>, std::vector<char>>> m_msgQ;
	std::mutex m_qLock;
	std::condition_variable m_qcv;

	PacketReader m_reader;

	LuaHelper* m_luaHelper;

	std::string m_gameServerIP;
	short m_gameServerPort;

	std::string m_DBServerIP;
	short m_DBServerPort;

	std::shared_ptr<Session> m_gameSession;
	std::shared_ptr<Session> m_dbSession;

	std::unordered_map<uint32_t, std::shared_ptr<Session>> m_sessionMap;
	static uint32_t m_sessionCount;

public:
	LoginProcessor(LuaHelper* _luaHelper);
	virtual ~LoginProcessor();

public:
	virtual bool Process() override;
	virtual void AddMsg(std::shared_ptr<Session> _session, std::vector<char>&& _buffer) override;
	virtual void Init() override;
	virtual void ConnectServer(std::shared_ptr<Server> _server) override;

private:
	std::string GetSaltedString(const std::string& _string, const std::string& _salt);

private:

	/// 여기서 부터 패킷 처리 함수
	void _CTL_RES_LOGIN(std::shared_ptr<Session> _session, CTL_RES_LOGIN&& _data);
	void _CTL_RES_ID_AVAILABLITY(std::shared_ptr<Session> _session, CTL_RES_ID_AVAILABLITY&& _data);
	void _DTL_ACK_LOGIN_DATA(DTL_ACK_LOGIN_DATA&& _data);
	void _DTL_ACK_ID_AVAILABLITY(DTL_ACK_ID_AVAILABLITY&& _data);
};