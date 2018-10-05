/* Copyright 2018 Ryan Cooper (RyanLoringCooper@gmail.com)
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "../NetworkConnection.h"

// protected
bool NetworkConnection::setupServer(const int &port) {
    WSADATA wsaData;
    connAddr = NULL;
    struct addrinfo *ptr = NULL, hints;
    ZeroMemory(&hints, sizeof (hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = connectionType;
    if(connectionType == SOCK_STREAM) {
        hints.ai_protocol = IPPROTO_TCP;
    } else {
        hints.ai_protocol = IPPROTO_UDP;
    }
    hints.ai_flags = AI_PASSIVE;
    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return false;
    }
    // Resolve the local address and port to be used by the server
    iResult = getaddrinfo(NULL, itoa(port), &hints, &connAddr);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return false;
    }
    // setup socket
    mSocket = socket(connAddr->ai_family, connAddr->ai_socktype, connAddr->ai_protocol);
    if (mSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(connAddr);
        freeaddrinfo(ptr);
        WSACleanup();
        return false;
    }   
    setBlocking(blockingTime);
    // Setup the TCP listening socket
    iResult = bind(mSocket, connAddr->ai_addr, (int)connAddr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(connAddr);
        freeaddrinfo(ptr);
        closesocket(mSocket);
        WSACleanup();
        return false;
    }
    // frees up memory related to connection information
    freeaddrinfo(connAddr);
    freeaddrinfo(ptr);
    return waitForClientConnection();
}

bool NetworkConnection::setupClient(const char *ipaddr, const int &port) {
    // Initialize Winsock
    WSADATA wsaData;
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", iResult);
        return return false;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_INET;
    hints.ai_socktype = connectionType;
    if(connectionType == SOCK_STREAM) {
        hints.ai_protocol = IPPROTO_TCP;
    } else {
        hints.ai_protocol = IPPROTO_UDP;
    }

    // Resolve the server address and port
    iResult = getaddrinfo(ipaddr, itoa(port), &hints, &connAddr);
    if ( iResult != 0 ) {
        fprintf(stderr, "getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return false;
    }
    mSocket = socket(connAddr->ai_family, connAddr->ai_socktype, connAddr->ai_protocol);
    if (mSocket == INVALID_SOCKET) {
        fprintf(stderr, "socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    setBlocking(blockingTime);
    return connectToServer();
}

bool NetworkConnection::connectToServer() { 
    iResult = connect(mSocket, connAddr->ai_addr, (int)connAddr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(mSocket);
        mSocket = INVALID_SOCKET;
        return false;
    }
    connected = true;
    return true;
}

bool NetworkConnection::waitForClientConnection() {
    // setup listening on socket for connections
    if (listen(mSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf( "Listen failed with error: %ld\n", WSAGetLastError() );
        closesocket(mSocket);
        WSACleanup();
        return false;
    }
    printf("Waiting for client connection...\n");
    // accept a client socket
    clientSocket = accept(mSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(mSocket);
        WSACleanup();
        return false;
    } 
    printf("Controller client connected!\n");
    connected = true;
    return true;
}

void NetworkConnection::exitGracefully() {
    // shutdown the send half of the connection since no more data will be sent
    if(connected) {
        if(clientSocket != INVALID_SOCKET) {
            int iResult = shutdown(clientSocket, SD_SEND);
            if (iResult == SOCKET_ERROR) {
                fprintf(stderr, "shutdown failed: %d\n", WSAGetLastError());
            }
            // cleanup
            closesocket(clientSocket);
        }
        closesocket(mSocket);
    }
    WSACleanup();
}

int NetworkConnection::getData(char *buff, const int &buffSize) { 
    if(connected) {
        if(clientSocket != INVALID_SOCKET) {
            return recv(clientSocket, buff, buffSize, 0);
        } else {
            return recv(mSocket, buff, buffSize, 0);
        }
    }
    return -1;
}

void NetworkConnection::failedRead() {
    if(connected) {
        closesocket(clientSocket);
        connected = false;
    }
    waitForClientConnection();
}

// public 
NetworkConnection::NetworkConnection(const NetworkConnection &other) : CommConnection(other) {
    if(this == &other) {
        return;
    }   
    *this = other;
}

NetworkConnection &NetworkConnection::operator=(const NetworkConnection &other) {
    if(this == &other) {
        return *this;
    }   
    mSocket = other.mSocket;
    clientSocket = other.clientSocket;
    memcpy(result, other.result, sizeof(addrinfo));
    CommConnection::operator=(other);
    return *this;
}

bool NetworkConnection::write(const char *buff, const int &buffSize) { 
	if(!connected) 
		return false;
    int iSendResult;
    if(clientSocket != INVALID_SOCKET) {
        iSendResult = send(clientSocket, buff, buffSize, 0);
    } else {
        iSendResult = send(mSocket, buff, buffSize, 0);
    if(iSendResult == SOCKET_ERROR) {
        fprintf(stderr, "Failed to send: %s", buff);
        return false;
    }
    return true;
}
