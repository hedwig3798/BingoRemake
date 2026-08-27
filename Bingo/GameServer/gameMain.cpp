#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

int main() 
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(8002);

	bind(server_fd, (struct sockaddr*)&address, sizeof(address));
	listen(server_fd, 3);

	std::cout << "[게임 서버] 8002번 포트 대기 중 (Windows)" << std::endl;

	while (true) 
	{
		SOCKET client_socket = accept(server_fd, nullptr, nullptr);
		std::cout << "[게임 서버] 클라이언트 접속. 게임 데이터 전송." << std::endl;

		const char* msg = "WELCOME_TO_GAME";
		send(client_socket, msg, strlen(msg), 0);
		closesocket(client_socket);
	}

	WSACleanup();
	return 0;
}