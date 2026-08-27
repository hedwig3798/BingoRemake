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

	IProcessor* m_processor;

public:
	Server(short _port);

public:
	void SetProcessor(IProcessor* _processor) { m_processor = _processor; };
	void Run();

private:
	void Accept();
};