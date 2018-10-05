/* Copyright 2018 Ryan Cooper (RyanLoringCooper@gmail.com)
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once
#ifndef NETWORKCONNECTION_H
#define NETWORKCONNECTION_H
#undef UNICODE

#if defined(__linux__) || defined(__linux) || defined(linux) 
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/types.h> 
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h> 
    #include <errno.h>
#elif defined(_WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    // Need to link with Ws2_32.lib
    #pragma comment (lib, "Ws2_32.lib")
    #pragma comment (lib, "Mswsock.lib")
    #pragma comment (lib, "AdvApi32.lib")
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
        struct sockaddr_in mAddr, rAddr;
#elif defined(_WIN32)
        SOCKET mSocket, clientSocket;
        struct addrinfo *result;
#else
#error Unsupported os
#endif
        int connectionType;
        bool server;

        bool setupServer(const int &port);
        bool setupClient(const char *ipaddr, const int &port);
        bool waitForClientConnection();
        bool connectToServer();

        void failedRead();
        int getData(char *buff, const int &buffSize);
        void exitGracefully();
        bool setBlocking(const int &blockingTime = -1);
    public:
        NetworkConnection(const int &port, const int &connectionType = SOCK_STREAM, const char *ipaddr = "", const int &blockingTime = -1, const bool &debug = false, const bool &noReads = false);
        NetworkConnection(const NetworkConnection &other);
        ~NetworkConnection();
        NetworkConnection &operator=(const NetworkConnection &other);
        using CommConnection::write;

        bool write(const char *buff, const int &buffSize);
};

#endif 
