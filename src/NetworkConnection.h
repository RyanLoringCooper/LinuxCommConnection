/* NetworkConnection.h -- Prototypes for class that connects to an ip based network
 * Copyright (C) 2018 Ryan Loring Cooper (RyanLoringCooper@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
