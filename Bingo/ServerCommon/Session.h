#pragma once

#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <queue>

#include "Serializer.h"
#include "Packet.h"
#include "IProcessor.h"

#define MAX_LENGHT 65536

class Session
	: public std::enable_shared_from_this<Session>
{
public:
	boost::asio::ip::tcp::socket m_socket;

	PacketWriter m_writer;
	std::vector<char> m_ringBuffer;
	uint16_t m_writePos;

	std::shared_ptr<IProcessor> m_processor;
	
private:
	std::queue<std::vector<char>> m_sendQ;
	std::mutex m_sendQMutex;
 
public:
	Session(boost::asio::ip::tcp::socket _socket, std::shared_ptr<IProcessor> _processor);
	virtual ~Session();

public:
	void Start();

	template <typename T>
	void SendPacket(const T& _packet)
	{
		Serialize(m_writer, _packet);
		SendPacket(std::move(m_writer.GetBuffer()));
	}

	void SendPacket(std::vector<char> _buffer);

private:
	void RecvPacket();
	void ProcessPacket();
	void DoAsyncSend();
};