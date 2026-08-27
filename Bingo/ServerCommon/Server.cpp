#include "Server.h"

Server::Server(short _port)
	: m_ioContext()
	, m_acceptor(m_ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), _port))
{
	Accept();
}

void Server::Run()
{
	m_ioContext.run();
}

void Server::Accept()
{
	m_acceptor.async_accept(
		[this](boost::system::error_code _ec, boost::asio::ip::tcp::socket _socket)
		{
			if (!_ec)
			{
				std::shared_ptr<Session> session = std::make_shared<Session>(std::move(_socket), m_processor);
				std::cout << "accept!" << std::endl;
				session->Start();
			}
			else
			{
				std::cout << "cannot accept client : " << _ec.message() << std::endl;
			}
			Accept();
		}
	);

}