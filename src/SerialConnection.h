#pragma once
#ifndef SERIALCONNECTION_H
#define SERIALCONNECTION_H

#if defined(__linux__) || defined(__linux) || defined(linux)

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#elif defined(_WIN32)

#include <windows.h>

#else
#error Unsupported os
#endif

#include <cstdio>
#include "CommConnection.h"

class SerialConnection : public CommConnection {
protected:
#if defined(__linux__) || defined(__linux) || defined(linux)
	int ser;
	int set_interface_attribs (const int &speed, const int &parity); 
	int set_blocking (const bool &should_block);
#elif defined(_WIN32)
    HANDLE handler;    
#else
#error Unsupported os
#endif

	void failedRead();
	int getData(char *buff, const int &buffSize);
	void exitGracefully();
    bool setBlocking(const int &blockingTime = -1);
public:
	SerialConnection(char *portName, const int &speed, const int &parity, const int &blockingTime = -1, const bool &noReads = false);
	~SerialConnection();
	
	bool write(const char *buff, const int &buffSize);
};

#endif // SERIALPORT_H
