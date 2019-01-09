// Transfer protocol (UDP)
#pragma once
#include "message.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <vector>
#pragma comment(lib, "wsock32.lib")

class ACExchangeInterface {
public:
	virtual int open(const char* adr, int port) = 0;
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

	int open(const char* adr, int port) {
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
		LOG("Closed socket");
		return 0;
	}
};

class CUdpClient : public CUdp {
public:
	int send(CMessage* message, void* address = nullptr, int size = 0) override {
		sendto(srSocket, message->msg, message->msgSize, NULL, (sockaddr*)&serverAddr, sizeof(serverAddr));
		return 0;
	}

	CMessage* recieve() override {
		u_long ioBuffSize;
		ioctlsocket(srSocket, FIONREAD, &ioBuffSize);
		LOG("Bytes in buffer = " << ioBuffSize);
		if (ioBuffSize <= 0) return NULL;
		sockaddr_in client;
		int clientSize = sizeof(client);
		int err = recvfrom(srSocket, recvbuf, buffSize, NULL, (sockaddr*)&client, &clientSize);
		if (err > 0) {
			LOG("Data recieved by client from " << inet_ntoa(client.sin_addr) << " : " << ntohs(client.sin_port));
		} else {
			LOG("ERROR on recieve(): " << WSAGetLastError());
		}
		return new CMessage(recvbuf, buffSize);
	}
};

class CUdpServer : public CUdp {
public:
	sockaddr_in clientAddr;

	int open(const char* adr, int port) override {// override ne override hz
		CUdp::open(adr, port);
		int err = bind(srSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
		if (err == SOCKET_ERROR) {
			LOG("Server bind ERROR: " << WSAGetLastError());
			close();
			return 1;
		}
		LOG("Server binded successfully");
		return 0;
	}

	int send(CMessage* message, void* address, int size) override {
		sendto(srSocket, message->msg, message->msgSize, NULL, (sockaddr*)address, size);
		return 0;
	}

	CMessage* recieve() override {
		u_long ioBuffSize;
		ioctlsocket(srSocket, FIONREAD, &ioBuffSize); // number of bytes in port buffer
		LOG("Bytes in buffer = " << ioBuffSize);
		if (ioBuffSize <= 0) return NULL;
		int clientSize = sizeof(clientAddr);
		int err = recvfrom(srSocket, recvbuf, buffSize, NULL, (sockaddr*)&clientAddr, &clientSize);
		if (err > 0) {
			LOG("Data recieved by server from " << inet_ntoa(clientAddr.sin_addr) << " : " << ntohs(clientAddr.sin_port));
		} else {
			LOG("ERROR on recieve(): " << WSAGetLastError());
		}
		return new CMessage(recvbuf, buffSize);
	}
};

class CUdpServerIdle : public CUdpServer {
public:
	CUdpServerIdle(CUdpServer* baseServer) {
		wsaData = baseServer->wsaData;
		srSocket = baseServer->srSocket;
		clientAddr = baseServer->clientAddr;
	}
};