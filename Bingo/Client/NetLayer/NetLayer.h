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
#include <afxdialogex.h>
#include <unordered_map>
#include "Serializer.h"
#include "Packet.h"

#pragma comment(lib, "ws2_32.lib")

#define MAX_LENGHT 65536

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
	std::queue<std::vector<char>> m_recvq;
	std::vector<char> m_ringBuffer;
	uint16_t m_writePos;

	std::thread m_packetProccessor;

	PacketReader m_reader;
	std::vector<char> m_readBuffer;

	bool m_endFlag;

	CDialogEx* m_dialog;

	std::unordered_map<uint32_t, uint32_t> m_waitingPacket;

public:
	void InitNetLayer();

	void SetDialog(CDialogEx* _dialog) { m_dialog = _dialog; };

	template <typename T>
	bool Send(const T& _data)
	{
		m_writer.GetBuffer().clear();
		Serialize(m_writer, _data);
		return Send(std::move(m_writer.GetBuffer()));
	}

	void SetWaitPacket(uint32_t _packetID, uint32_t _time);
	bool IsWaitingPacket(uint32_t _packetID);
	void RelaseWaiting(uint32_t _packetID);

private:
	void ConnectServer(const char* _host, const char* _port);
	void ConnectClose();

	bool Send(std::vector<char>&& _buffer);

	void SnedThread();
	void RecvThread();
	void PacketReader();
	void ProcessPacket();

	void BroadcastPacket();
};