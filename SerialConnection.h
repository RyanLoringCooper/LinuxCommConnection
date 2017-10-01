#pragma once
#ifndef SERIALCONNECTION_H
#define SERIALCONNECTION_H

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstdio>
#include "CommConnection.h"

class SerialConnection : public CommConnection {
protected:
	int ser;

	void failedRead();
	int getData(char *buff, const int &buffSize);
	void exitGracefully();
	int set_interface_attribs (const int &speed, const int &parity); 
	int set_blocking (const bool &should_block);
public:
	SerialConnection();
	SerialConnection(char *portName);
	~SerialConnection();
	bool write(char *buff, const int &buffSize);
};

#endif // SERIALPORT_H