#pragma once
#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H
#undef UNICODE

#if defined(__linux__) || defined(__linux) || defined(linux) 

#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#elif defined(_WIN32)

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#else
#error Unsupported os
#endif

#include <cstdlib>
#include <cstdio>
#include "CommConnection.h"

class NetworkConnection : public CommConnection {
protected:
#if defined(__linux__) || defined(__linux) || defined(linux) 
	int mSocket, clientSocket;
	struct sockaddr_in connAddr;
#elif defined(_WIN32)
	SOCKET listenSocket, clientSocket;
#else
#error Unsupported os
#endif
	int connectionType;

	bool setupServer(const int &port);
	bool setupClient(const char *ipaddr, const int &port);
	bool waitForClientConnection();
	bool connectToServer();
	void exitGracefully();

	void failedRead();
	int getData(char *buff, const int &buffSize);
public:
	NetworkConnection(const int &port, const int &connectionType = SOCK_STREAM, const char *ipaddr = 0);
	~NetworkConnection();
	
	bool write(char *buff, const int &buffSize);
};

#endif 
