#pragma once

#include <iostream>
#include <memory>
#include <boost/asio.hpp>
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

	IProcessor* m_processor;

public:
	Session(boost::asio::ip::tcp::socket _socket, IProcessor* _processor);
	virtual ~Session();

public:
	void Start();

private:
	void RecvPacket();
	void ProcessPacket();
	void SendPacket(std::size_t _length);
};