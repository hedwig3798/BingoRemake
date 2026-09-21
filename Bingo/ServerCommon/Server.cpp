#include "Server.h"

Server::Server(short _port)
	: m_ioContext()
	, m_acceptor(m_ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), _port))
	, m_timer(m_ioContext, std::chrono::seconds(1))
{
	Accept();
}

void Server::Run()
{
	m_ioContext.run();
}

std::shared_ptr<Session> Server::ConnectServer(const std::string& _ip, short _port)
{
	boost::asio::ip::tcp::socket socket(m_ioContext);
	boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address(_ip), _port);

	boost::system::error_code ec;
	socket.connect(endpoint, ec);

	if (ec)
	{
		std::cerr << _ip  << ":" << _port << " 연결 실패: " << ec.message() << std::endl;
		return nullptr;
	}

	std::shared_ptr<Session> session = std::make_shared<Session>(std::move(socket), m_processor, m_ioContext);
	session->Start();
	return session;
}

void Server::Accept()
{
	m_acceptor.async_accept(
		[this](boost::system::error_code _ec, boost::asio::ip::tcp::socket _socket)
		{
			if (!_ec)
			{
				std::shared_ptr<Session> session = std::make_shared<Session>(std::move(_socket), m_processor, m_ioContext);
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

void Server::Tick()
{
	m_timer.expires_after(std::chrono::seconds(1));

	m_timer.async_wait([this](const boost::system::error_code& _ec)
		{
			m_processor->Tick();
		}
	);
}
