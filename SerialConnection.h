#pragma once
#ifndef SERIALCONNECTION_H
#define SERIALCONNECTION_H

#include <termios.h>
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
public:
	SerialConnection();
	SerialConnection(char *portName);
	~SerialConnection();
	bool write(char *buff, const int &buffSize);
};

#endif // SERIALPORT_H