/* Copyright 2018 Ryan Cooper (RyanLoringCooper@gmail.com)
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
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
	SerialConnection(const char *portName, const int &speed, const int &parity, const int &blockingTime = -1, const bool &debug = false, const bool &noReads = false);
	SerialConnection(const SerialConnection &other);
	SerialConnection &operator=(const SerialConnection &other);
	~SerialConnection();
	using CommConnection::write;
	// returns false and sets errno upon error
	bool write(const char *buff, const int &buffSize);
};

#endif // SERIALPORT_H
