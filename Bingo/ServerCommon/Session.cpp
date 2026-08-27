#include "Session.h"
#include <iterator>

Session::Session(boost::asio::ip::tcp::socket _socket, std::shared_ptr<IProcessor> _processor)
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

void Session::SendPacket(std::vector<char> _buffer)
{
	{
		std::lock_guard<std::mutex> lock(m_sendQMutex);
		m_sendQ.push(std::move(_buffer));
	}

	DoAsyncSend();
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
	// 비동기 동작 중 소멸 방지
	auto self(shared_from_this());

	// 현재 읽은 위치
	uint16_t readPos = 0;
	while (true)
	{
		// 남은 바이트 수
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

		// 프로세서에게 메세지 전달
		m_processor->AddMsg(
			self
			, std::vector<char>(
				std::make_move_iterator(m_ringBuffer.data() + readPos)
				, std::make_move_iterator(m_ringBuffer.data() + readPos + header->m_size)
			)
		);

		// 읽은 위치 증가
		readPos += header->m_size;
	}

	// 남은 데이터를 벡터 앞으로 이동
	uint16_t leftover = m_writePos - readPos;
	if (leftover > 0 && readPos > 0)
	{
		std::memmove(m_ringBuffer.data(), m_ringBuffer.data() + readPos, leftover);
	}

	// 읽어야 할 위치 동기화
	m_writePos = leftover;
}

void Session::DoAsyncSend()
{
	// 비동기 동작 중 소멸 방지
	auto self(shared_from_this());

	// 외부 락
	std::unique_lock<std::mutex> qlock(m_sendQMutex);

	// 큐가 비어있으면 리턴
	if (true == m_sendQ.empty())
	{
		return;
	}

	// 비동기 전송
	boost::asio::async_write(
		m_socket,
		boost::asio::buffer(m_sendQ.front()),
		[this, self](boost::system::error_code _ec, std::size_t _length)
		{
			// 에러 처리
			if (_ec)
			{
				std::cout << "Send Error: " << _ec.message() << std::endl;
				return;
			}

			// 내부 락
			bool hasMore = false;
			{
				std::lock_guard<std::mutex> lock(m_sendQMutex);
				m_sendQ.pop();
				hasMore = !m_sendQ.empty();
			}

			// 더 보낼 내용이 있다면 재귀
			if (hasMore)
			{
				DoAsyncSend();
			}
		}
	);
	// 외부 락 해제
	qlock.unlock();
}

