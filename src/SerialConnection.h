/* SerialConnection.h -- Prototypes for class that connects to a serial device
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
