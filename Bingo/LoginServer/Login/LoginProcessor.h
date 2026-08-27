#pragma once

#include "IProcessor.h"
#include "Packet.h"
#include "Serializer.h"
#include "luaHelper.h"
#include <memory>
#include <queue>
#include <mutex>

class LoginProcessor
	: public IProcessor
{
private:
	std::queue<std::pair<std::shared_ptr<Session>, std::vector<char>>> m_msgQ;
	std::mutex m_qLock;

	PacketReader m_reader;

	LuaHelper* m_luaHelper;

	std::string m_gameServerIP;
	short m_gameServerPort;

public:
	LoginProcessor(LuaHelper* _luaHelper);
	virtual ~LoginProcessor();

public:
	virtual bool Process() override;
	virtual void AddMsg(std::shared_ptr<Session> _session, std::vector<char>&& _buffer) override;
	virtual void Init() override;

private:


private:

	/// 여기서 부터 패킷 처리 함수
	void _LOGIN_PACKET_SEND(std::shared_ptr<Session> _session, LOGIN_PACKET_SEND&& _data);
};