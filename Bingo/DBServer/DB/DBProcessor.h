#pragma once

#include "IProcessor.h"
#include "Packet.h"
#include "Serializer.h"
#include "luaHelper.h"
#include <memory>
#include <queue>
#include <mutex>
#include <libpq-fe.h>
#include <boost/asio.hpp>
#include "Session.h"

struct DBConnection
{
	PGconn* m_apiConnection;
	boost::asio::ip::tcp::socket m_socket;
};

class DBProcessor
	: public IProcessor
{
private:
	std::queue<std::pair<std::shared_ptr<Session>, std::vector<char>>> m_msgQ;
	std::mutex m_qLock;
	std::condition_variable m_qcv;

	PacketReader m_reader;

	LuaHelper* m_luaHelper;

	boost::asio::io_context m_dbIoContext;
	boost::asio::executor_work_guard<boost::asio::io_context::executor_type> m_workGuard;
	std::thread m_dbIoThread;

	std::queue<DBConnection*> m_connQ;
	std::mutex m_connQMutex;
	std::condition_variable m_connQCV;
	int m_DBConnectionCount;
	std::string m_DBURL;

	std::shared_ptr<Session> m_loginSession;
	std::shared_ptr<Session> m_gameSession;

public:
	DBProcessor(LuaHelper* _luaHelper);
	virtual ~DBProcessor();

public:
	virtual bool Process() override;
	virtual void AddMsg(std::shared_ptr<Session> _session, std::vector<char>&& _buffer) override;
	virtual void Init() override;
	virtual void ConnectServer(std::shared_ptr<Server> _server) override;

private:
	void ReturnConnection(DBConnection* conn);
	DBConnection* GetConnection();

private:

	/// 여기서 부터 패킷 처리 함수
	void _LTD_RES_LOGIN_DATA(std::shared_ptr<Session> _session, LTD_RES_LOGIN_DATA&& _data);
	void _DTL_ACK_LOGIN_DATA(std::shared_ptr<Session> _session, DBConnection* _conn, std::string _id, std::string _hashedPW, uint32_t _sesstionCount);

	void _LTD_RES_ID_AVAILABLITY(std::shared_ptr<Session> _session, LTD_RES_ID_AVAILABLITY&& _data);
	void _DTL_ACK_ID_AVAILABLITY(std::shared_ptr<Session> _session, DBConnection* _conn, uint32_t _sesstionCount);
};