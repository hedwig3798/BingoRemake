#include "NetLayer.h"
#include "MassageDefine.h"

NetLayer::NetLayer()
	: m_sock(0)
	, m_reader()
	, m_readBuffer(65536)
	, m_ringBuffer(65536)
	, m_dialog(nullptr)
	, m_writePos(0)
{

}

NetLayer::~NetLayer()
{
	m_endFlag = true;
	m_sendCV.notify_all();

	ConnectClose();
	WSACleanup();

	m_recvThread.join();
	m_sendThread.join();
	m_packetProccessor.join();
}

void NetLayer::InitNetLayer()
{
	m_endFlag = false;

	WSADATA wsaData;
	if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		std::cerr << "WSAStartup 초기화 실패" << std::endl;
		return;
	}

	ConnectServer("127.0.0.1", "8001");

	if (0 != m_sock && INVALID_SOCKET != m_sock)
	{
		m_sendThread = std::thread(&NetLayer::SnedThread, this);
		m_recvThread = std::thread(&NetLayer::RecvThread, this);
		m_packetProccessor = std::thread(&NetLayer::BroadcastPacket, this);
	}
}

void NetLayer::ConnectServer(const char* _host, const char* _port)
{
	addrinfo hints = { 0 };
	addrinfo* res = nullptr;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int iResult = getaddrinfo(_host, _port, &hints, &res);
	if (0 != iResult)
	{
		std::cerr << _host << " 주소 찾기 실패 : " << iResult << std::endl;
		return;
	}

	m_sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (SOCKET_ERROR == connect(m_sock, res->ai_addr, (int)res->ai_addrlen))
	{
		std::cerr << _host << " 연결 실패" << std::endl;
		closesocket(m_sock);
		freeaddrinfo(res);
		return;
	}

	std::cout << _host << ":" << _port << "연결 성공" << std::endl;
}

void NetLayer::ConnectClose()
{
	closesocket(m_sock);
}

bool NetLayer::Send(std::vector<char>&& _buffer)
{
	if (true == _buffer.empty())
	{
		return false;
	}

	{
		std::lock_guard<std::mutex> lock(m_sendLock);
		m_sendq.push(std::move(_buffer));
	}

	m_sendCV.notify_one();

	return true;
}

void NetLayer::SetWaitPacket(uint32_t _packetID, uint32_t _time)
{
	if (false == IsWaitingPacket(_packetID))
	{
		m_waitingPacket[_packetID] = _time;
	}
}

bool NetLayer::IsWaitingPacket(uint32_t _packetID)
{
	return m_waitingPacket.find(_packetID) != m_waitingPacket.end();
}

void NetLayer::RelaseWaiting(uint32_t _packetID)
{
	if (true == IsWaitingPacket(_packetID))
	{
		m_waitingPacket.erase(_packetID);
	}
}

void NetLayer::SnedThread()
{
	while (false == m_endFlag)
	{
		std::vector<char> buffer;

		{
			std::unique_lock<std::mutex> lock(m_sendLock);
			m_sendCV.wait(lock, [this]()
				{
					return (false == m_sendq.empty()) || (true == m_endFlag);
				});

			if (true == m_endFlag && true == m_sendq.empty())
			{
				break;
			}

			buffer = std::move(m_sendq.front());
			m_sendq.pop();
		}

		int size = buffer.size();
		int sendSize = 0;
		int leftSize = size;

		while (sendSize < size)
		{
			std::cout << "send Packet\n";

			int sended = send(m_sock, buffer.data() + sendSize, leftSize, 0);
			if (SOCKET_ERROR == sended)
			{
				return;
			}

			sendSize += sended;
			leftSize -= sended;
		}
	}
}

void NetLayer::RecvThread()
{
	while (false == m_endFlag)
	{
		// 일단 받아오기
		int recved = recv(m_sock, m_ringBuffer.data() + m_writePos, m_ringBuffer.size() - m_writePos, 0);
		
		// 에러 있으면 스레드 종료
		if (SOCKET_ERROR == recved)
		{
			int nErrorCode = WSAGetLastError();
			// 나중에 처리하기
			std::cerr << "something worng : " << nErrorCode << std::endl;
			break;
		}
		m_writePos += recved;
		ProcessPacket();
	}
}

void NetLayer::ProcessPacket()
{
	uint16_t readPos = 0;
	while (true)
	{
		// 남은 바이트 수
		uint16_t remainByte = m_writePos - readPos;

		// 쌓인 데이터가 헤더보자 작으면 다시 수신
		if (remainByte < sizeof(PacketHeader) || true == m_endFlag)
		{
			break;
		}
		PacketHeader* header = reinterpret_cast<PacketHeader*>(m_ringBuffer.data() + readPos);

		// 쌓인 데이터가 크기만큼 있어야 한다.
		if (remainByte < header->m_size)
		{
			break;
		}


		{
			std::unique_lock<std::mutex> lock(m_recvLock);
			m_recvq.push(
				std::vector<char>(
					std::make_move_iterator(m_ringBuffer.data() + readPos)
					, std::make_move_iterator(m_ringBuffer.data() + readPos + header->m_size)
				)
			);
		}

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

void NetLayer::BroadcastPacket()
{
	{
		std::unique_lock<std::mutex> lock(m_recvLock);
		while (true)
		{
			if (true == m_endFlag)
			{
				break;
			}

			if (false == lock.owns_lock())
			{
				lock.lock();
			}

			if (true == m_recvq.empty())
			{
				lock.unlock();
				continue;
			}

			std::vector<char> packet = std::move(m_recvq.front());
			m_recvq.pop();
			lock.unlock();

			PacketHeader* header = reinterpret_cast<PacketHeader*>(packet.data());
			switch (header->m_ID)
			{
			case LTC_ACK_LOGIN::PACKET_ID:
			{
				LTC_ACK_LOGIN* ackData = new LTC_ACK_LOGIN();
				m_reader.SetBuffer(packet, sizeof(PacketHeader));
				Deserialize(m_reader, *ackData);
				m_dialog->PostMessage(CM_LTC_ACK_LOGIN, (WPARAM)ackData);
				break;
			}
			case LTC_ACK_ID_AVAILABLITY::PACKET_ID:
			{
				LTC_ACK_ID_AVAILABLITY* ackData = new LTC_ACK_ID_AVAILABLITY();
				m_reader.SetBuffer(packet, sizeof(PacketHeader));
				Deserialize(m_reader, *ackData);
				m_dialog->PostMessage(CM_LTC_ACK_ID_AVAILABLITY, (WPARAM)ackData);
				break;
			}
			case LTC_ACK_SING_UP::PACKET_ID:
			{
				LTC_ACK_SING_UP* ackData = new LTC_ACK_SING_UP();
				m_reader.SetBuffer(packet, sizeof(PacketHeader));
				Deserialize(m_reader, *ackData);
				m_dialog->PostMessage(CM_LTC_ACK_SING_UP, (WPARAM)ackData);
				break;
			}
			default:
				break;
			}
		}
	}
}

