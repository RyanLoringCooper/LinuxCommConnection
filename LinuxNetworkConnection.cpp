#include <errno.h>
#include "NetworkConnection.h"

// protected
bool NetworkConnection::setupServer(const int &port) {
	bzero((char *) &connAddr, sizeof(struct sockaddr_in));
	connAddr.sin_family = AF_INET;
	connAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	connAddr.sin_port = htons(port);
	mSocket = socket(connAddr.sin_family, connectionType, 0);
	if (mSocket < 0) {
		fprintf(stderr, "ERROR opening socket: %d\n", errno);
		return false;
	}
	if (bind(mSocket, (struct sockaddr *) &connAddr, sizeof(connAddr)) < 0) {
		fprintf(stderr, "ERROR on binding to port %d. Is it already taken?\n", port);
		return false;
	} 
	if(connectionType == SOCK_STREAM) {
		return waitForClientConnection();
	} else {
		// UDP connection
		connected = true;
		printf("Successfully setup socket server.\n");
		return true;
	}
}

bool NetworkConnection::setupClient(const char *ipaddr, const int &port) {
	struct hostent *server = gethostbyname(ipaddr);
	bzero((char *) &connAddr, sizeof(connAddr));
    connAddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&connAddr.sin_addr, server->h_length);
	connAddr.sin_port = htons(port);
	mSocket = socket(connAddr.sin_family, connectionType, 0);
	if(connectionType == SOCK_STREAM) {
		return connectToServer();
	} else {
		// UDP connection
		connected = true;
		return true;
	}
}

bool NetworkConnection::connectToServer() {
	while(connect(mSocket,(struct sockaddr *) &connAddr,sizeof(connAddr)) < 0) {
        fprintf(stderr, "Couldn't connect to server. Will retry in a second.");
        std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	connected = true;
    return true;
}

bool NetworkConnection::waitForClientConnection() {
	bzero((char *) &connAddr, sizeof(connAddr));
	socklen_t connLen = sizeof(connAddr);
	listen(mSocket,5);
	printf("Waiting for controller client connection...\n");
	// accept a client socket
	clientSocket = accept(mSocket, (struct sockaddr *) &connAddr, &connLen);
	if (clientSocket < 0) {
		fprintf(stderr, "Accepting a connection failed with errno %d\n", errno);
		close(mSocket);
		return false;
	} else {
		printf("IPv4 client connected!\n");
		connected = true;
		return true;
	}
}

void NetworkConnection::exitGracefully() {
	// shutdown the send half of the connection since no more data will be sent
	// cleanup
	if(clientSocket > 0)
		close(clientSocket);
	if(mSocket > 0)
		close(mSocket);
}

int NetworkConnection::getData(char *buff, const int &buffSize) {
	if(connected && !interruptRead) {
		if(clientSocket > 0) {
			return recv(clientSocket, buff, buffSize, 0);
		} else {
			socklen_t len = sizeof(connAddr);
			return recvfrom(mSocket, buff, buffSize, 0, (struct sockaddr *)&connAddr, &len);
		}
	}
	return -1;
}

void NetworkConnection::failedRead() {
	connected = false;
	if(connectionType == SOCK_STREAM) {
		if(clientSocket > 0) {
			close(clientSocket);
			waitForClientConnection();
		} else {
			connectToServer();
		}
	}
}

// public 
bool NetworkConnection::write(const char *buff, const int &buffSize) {
	if(!connected) 
		return false;
	if(clientSocket > 0) {
		return sendto(clientSocket, buff, buffSize, 0, (sockaddr *) &connAddr, (socklen_t)sizeof(connAddr)) >= 0;
	} else {
		return sendto(mSocket, buff, buffSize, 0, (sockaddr *) &connAddr, (socklen_t)sizeof(connAddr)) >= 0;
	}
}
