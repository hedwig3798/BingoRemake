#include "NetLayer.h"

NetLayer::NetLayer()
	: m_sock(0)
	, m_reader()
	, m_readBuffer(1024)
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
	}
}


void NetLayer::Process()
{

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
	uint16_t recvLeft = 0;
	std::vector<char> dataBuffer;
	std::vector<char> headerBuffer(sizeof(PacketHeader));

	uint16_t headerRecvLeft = sizeof(PacketHeader);
	PacketHeader* headerData;

	while (false == m_endFlag)
	{
		// 일단 받아오기
		int recved = recv(m_sock, headerBuffer.data(), sizeof(PacketHeader), MSG_WAITALL);

		// 에러 있으면 스레드 종료
		if (SOCKET_ERROR == recved)
		{
			// 나중에 처리하기
			std::cerr << "something worng" << std::endl;
			break;
		}
		if (sizeof(PacketHeader) != recved)
		{
			std::cerr << "header read fail" << std::endl;
			continue;
		}

		headerData = reinterpret_cast<PacketHeader*>(headerBuffer.data());
		headerData->m_size -= sizeof(PacketHeader);
		dataBuffer.resize(headerData->m_size);

		recved = recv(m_sock, dataBuffer.data(), headerData->m_size, MSG_WAITALL);
		if (headerData->m_size != recved)
		{
			std::cerr << "data read fail" << std::endl;
			continue;
		}

		{
			std::unique_lock<std::mutex> lock(m_recvLock);
			m_recvq.push({ headerData->m_ID, std::move(dataBuffer) });
		}

	}
}

