#pragma once
#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H
#undef UNICODE

#include <cstdlib>
#include <cstdio>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "CommConnection.h"

class NetworkConnection : public CommConnection {
protected:
	int mSocket, clientSocket, connectionType;
	struct sockaddr_in connAddr;

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