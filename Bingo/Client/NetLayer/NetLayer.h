#pragma once
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <windows.h>  
#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "Serializer.h"
#include "Packet.h"

#pragma comment(lib, "ws2_32.lib")

class NetLayer
{
public:
	NetLayer();
	virtual ~NetLayer();

private:
	SOCKET m_sock;

	std::thread m_sendThread;
	std::queue<std::vector<char>> m_sendq;
	std::mutex m_sendLock;
	std::condition_variable m_sendCV;
	PacketWriter m_writer;

	std::thread m_recvThread;
	std::mutex m_recvLock;
	std::queue<std::pair<uint32_t, std::vector<char>>> m_recvq;

	PacketReader m_reader;
	std::vector<char> m_readBuffer;

	bool m_endFlag;

public:
	void InitNetLayer();

	template <typename T>
	bool Send(const T& _data)
	{
		m_writer.GetBuffer().clear();
		Serialize(m_writer, _data);
		return Send(std::move(m_writer.GetBuffer()));
	}

private:
	void ConnectServer(const char* _host, const char* _port);
	void ConnectClose();

	bool Send(std::vector<char>&& _buffer);

	void SnedThread();
	void RecvThread();
};