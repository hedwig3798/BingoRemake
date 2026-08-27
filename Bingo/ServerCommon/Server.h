#pragma once

#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include "Session.h"
#include "IProcessor.h"

class Server
{
private:
	boost::asio::io_context m_ioContext;
	boost::asio::ip::tcp::acceptor m_acceptor;

	std::shared_ptr<IProcessor> m_processor;

public:
	Server(short _port);

public:
	void SetProcessor(std::shared_ptr<IProcessor> _processor) { m_processor = _processor; };
	void Run();
	std::shared_ptr<Session> ConnectServer(const std::string& _ip, short _port);

private:
	void Accept();
};