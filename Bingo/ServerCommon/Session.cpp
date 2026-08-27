#include "Session.h"
#include <iterator>

Session::Session(boost::asio::ip::tcp::socket _socket, IProcessor* _processor)
	: m_socket(std::move(_socket))
	, m_ringBuffer(MAX_LENGHT)
	, m_writePos(0)
	, m_processor(_processor)
{

}

Session::~Session()
{

}

void Session::Start()
{
	RecvPacket();
}

void Session::RecvPacket()
{
	// 비동기 콜백에서 객체가 살아있기 위함
	auto self(shared_from_this());

	m_socket.async_read_some(
		boost::asio::buffer(m_ringBuffer.data() + m_writePos, m_ringBuffer.size() - m_writePos)
		, [this, self](boost::system::error_code _ec, std::size_t _lenght)
		{
			if (_ec)
			{
				std::cout << "Disconnect: " << _ec.message() << std::endl;
				return;
			}

			m_writePos += _lenght;

			ProcessPacket();

			RecvPacket();
		}
	);
}

void Session::ProcessPacket()
{
	auto self(shared_from_this());

	uint16_t readPos = 0;
	while (true)
	{
		uint16_t remainByte = m_writePos - readPos;

		// 쌓인 데이터가 헤더보자 작으면 다시 수신
		if (remainByte < sizeof(PacketHeader))
		{
			break;
		}
		PacketHeader* header = reinterpret_cast<PacketHeader*>(m_ringBuffer.data() + readPos);

		// 쌓인 데이터가 크기만큼 있어야 한다.
		if (remainByte < header->m_size)
		{
			break;
		}

		m_processor->AddMsg(
			self
			, std::vector<char>(
				std::make_move_iterator(m_ringBuffer.data() + readPos)
				, std::make_move_iterator(m_ringBuffer.data() + readPos + header->m_size)
			)
		);

		readPos += header->m_size;
	}

	uint16_t leftover = m_writePos - readPos;
	if (leftover > 0 && readPos > 0)
	{
		std::memmove(m_ringBuffer.data(), m_ringBuffer.data() + readPos, leftover);
	}

	m_writePos = leftover;
}

void Session::SendPacket(std::size_t _length)
{
	// 비동기 콜백에서 객체가 살아있기 위함
	auto self(shared_from_this());
}

