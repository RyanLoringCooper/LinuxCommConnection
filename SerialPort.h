#pragma once
#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <windows.h>
#include <cstdio>
#include "CommConnection.h"

class SerialPort : public CommConnection {
protected:
	HANDLE handler;

	void failedRead();
	int getData(char *buff, const int &buffSize);
	void exitGracefully();
public:
	SerialPort();
	SerialPort(char *portName);
	~SerialPort();
	bool write(char *buff, const int &buffSize);
};

#endif // SERIALPORT_H