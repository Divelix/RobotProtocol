#pragma once
#include "message.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <vector>
#pragma comment(lib, "wsock32.lib")

class ACExchangeInterface {
	virtual int open(const char adr[], int port) = 0;
	virtual int close() = 0;
	virtual int send(CMessage* msg, void* address = nullptr, int size = 0) = 0;
	virtual CMessage* recieve() = 0;
};

class CUdp : ACExchangeInterface {
public:
	WSADATA wsaData;
	SOCKET srSocket;
	sockaddr_in serverAddr; // адрес сервера
	const int buffSize = 1024; // размер буфера
	char recvbuf[1024]; // буфер приема

	void setRecieveTimeout(int timeout) { setsockopt(srSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)); }
	void setSendTimeout(int timeout) { setsockopt(srSocket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)); }

	int open(const char adr[], int port) override {
		// Initialize Winsock
		WSAStartup(MAKEWORD(2, 2), &wsaData);

		// Create a SOCKET
		srSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = inet_addr(adr);
		serverAddr.sin_port = htons(port);
		return 0;
	}

	int close() override {
		closesocket(srSocket);
		WSACleanup();
		std::cout << "Closed socket" << std::endl;
		return 0;
	}

	int send(CMessage* message, void* address = nullptr, int size = 0) override {
		sendto(srSocket, message->msg, message->msgSize, NULL, (sockaddr*)&serverAddr, sizeof(serverAddr));
		return 0;
	}

	CMessage* recieve() override {
		int err = recvfrom(srSocket, recvbuf, buffSize, NULL, NULL, NULL);
		if (err > 0) {
			std::cout << "Buffer recieved" << std::endl;
		}
		else {
			std::cout << "ERROR: Buffer not recieved" << std::endl;
		}
		CMessage* message = new CMessage(recvbuf, buffSize);
		return message;
	}
};

class CUdpClient : public CUdp {

};

class CUdpServer : public CUdp {
public:
	std::vector<sockaddr_in> clients;

	int open(const char adr[], int port) override {
		CUdp::open(adr, port);
		int err = bind(srSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
		if (err == SOCKET_ERROR) {
			std::cout << "Server bind ERROR: " << WSAGetLastError() << std::endl;
			close();
			return 1;
		}
		std::cout << "Server binded successfully" << std::endl;
		return 0;
	}

	int send(CMessage* message, void* address, int size) override {
		sendto(srSocket, message->msg, message->msgSize, NULL, (sockaddr*)address, size);
		return 0;
	}

	CMessage* recieve() override {
		sockaddr_in client;
		int clientSize = sizeof(client);
		int err = recvfrom(srSocket, recvbuf, buffSize, NULL, (sockaddr*)&client, &clientSize);
		if (err > 0) {
			clients.push_back(client);
			std::cout << "Data recieved by server from " << inet_ntoa(client.sin_addr) << " : " << ntohs(client.sin_port) << std::endl;
		}
		else {
			std::cout << "ERROR on recieve(): " << WSAGetLastError() << std::endl;
		}
		CMessage* message = new CMessage(recvbuf, buffSize);
		return message;
	}
};
