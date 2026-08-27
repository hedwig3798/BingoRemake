#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <windows.h>  
#include <string>
#include "Login/LoginApp.h"

#pragma comment(lib, "ws2_32.lib")



CLoginApp theApp;

void ConntecServer(const char* _host, const char* _port, std::string _data, std::string& _recv) 
{
	addrinfo hints = { 0 }, * res = nullptr;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (0 != getaddrinfo(_host, _port, &hints, &res)) 
	{
		std::cerr << _host << " 주소 찾기 실패" << std::endl;
		return;
	}

	SOCKET sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (SOCKET_ERROR == connect(sock, res->ai_addr, (int)res->ai_addrlen))
	{
		std::cerr << _host << " 연결 실패" << std::endl;
		closesocket(sock);
		freeaddrinfo(res);
		return;
	}

	char buffer[1024] = { 0 };
	recv(sock, buffer, 1024, 0);
	std::cout << "-> 서버 응답 (" << _host << "): " << buffer << std::endl;

	closesocket(sock);
	freeaddrinfo(res);
}

int main() 
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	while (true)
	{
		std::string data;
		std::string recv;
		std::cout << "input : ";
		std::cin >> data;

		std::cout << "\n[진행 1] 로그인 서버에 접속 시도..." << std::endl;
		ConntecServer("127.0.0.1", "8001", data, recv);

		std::cout << "\n[진행 2] 게임 서버에 접속 시도..." << std::endl;
		ConntecServer("127.0.0.1", recv.c_str(), data, recv);

		std::cout << "\n[클라이언트] 모든 테스트 완료." << std::endl;
	}

	WSACleanup();
	return 0;
}




