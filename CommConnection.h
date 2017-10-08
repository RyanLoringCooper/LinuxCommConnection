#pragma once
#ifndef COMMCONNECTION_H
#define COMMCONNECTION_H

#include <thread>
#include <chrono>
#include <cstring>

#define MAX_DATA_LENGTH 32
#define BUFFER_SIZE 4096

class CommConnection {
protected:
	char *buffer;
	int readIndex, writeIndex, blockingTime;
	volatile bool connected, interruptRead;
    // blockingTime is in milliseconds
	bool noReads, begun, terminated;
	std::thread *readThread;

	void performReads();
	void fillBuffer(char *buff, const int &bytesRead);
	void closeThread();

	virtual void failedRead() = 0;
	virtual int getData(char *buff, const int &buffSize) = 0;
	virtual void exitGracefully() = 0;
public:
	CommConnection(const int &blockingTime = -1, const bool &noReads = false);

	bool begin();
	int available() const;
	char read();
	void read(char *buff, const unsigned int &bytesToRead);
	int readUntil(char *buff, const int &buffSize, const char &delim);
	bool isConnected() const;
	void clearBuffer();	
	void terminate();

    virtual ~CommConnection();
	virtual bool write(const char *buff, const int &bufSize) = 0;
};

#endif //COMMCONNECTION_H
