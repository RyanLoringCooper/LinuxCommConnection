#pragma once
#ifndef COMMCONNECTION_H
#define COMMCONNECTION_H

#include <thread>
#include <chrono>
#include <cstring>
#include <string>
#include <mutex>
#include <condition_variable>

#define MAX_DATA_LENGTH 32
#define BUFFER_SIZE 4096

class CommConnection {
protected:
	char *buffer;
	int readIndex, writeIndex, blockingTime;
	volatile bool connected, interruptRead;
    // blockingTime is in milliseconds
	bool noReads, begun, terminated, cvBool, debug;
	std::thread *readThread;
	std::mutex dataMutex;
	std::condition_variable cv;

	void performReads();
	void fillBuffer(char *buff, const int &bytesRead);
	void closeThread();

	virtual void failedRead() = 0;
	virtual int getData(char *buff, const int &buffSize) = 0;
	virtual void exitGracefully() = 0;
    virtual bool setBlocking(const int &blockingTime = -1) = 0;
public:
	CommConnection(const int &blockingTime = -1, const bool &debug = false, const bool &noReads = false);

	bool begin();
	int available() const;
	int waitForData();
	char read();
	void read(char *buff, const unsigned int &bytesToRead);
	int readUntil(char *buff, const int &buffSize, const char &delim);
    std::string readString(const unsigned int &bytesToRead = -1);
	bool isConnected() const;
	void clearBuffer();	
	void terminate();
    bool write(const std::string &buff);

    virtual ~CommConnection();
	virtual bool write(const char *buff, const int &bufSize) = 0;
};

#endif //COMMCONNECTION_H
