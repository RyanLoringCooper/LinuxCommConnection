/* CommConnection.h -- Prototypes for base class 
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
#ifndef COMMCONNECTION_H
#define COMMCONNECTION_H

#include <thread>
#include <chrono>
#include <cstring>
#include <string>
#include <mutex>
#include <condition_variable>

// size of the buffer that is filled when a read is preformed
#define _MAX_DATA_LENGTH 4096
// size of the circular buffer that the user is served data from
// 4194304 = 2^22 = 4MB
#define _BUFFER_SIZE 4194304

class CommConnection {
protected:
	// a circular buffer that holds the data read from a connection until the user requests it
	char *buffer;
	// indexes related to buffer. The readIndex cannot pass the writeIndex.
	int readIndex, writeIndex;
	// the amount of time preformReads() will wait before trying to read again
	// if less than 0, it will block indefinitely
	// if equal to 0, it will never block and will continuously try to read data
	// if more than 0, it will wait that long in milliseconds
	int blockingTime;
	// flag to indicate if the connection is connected. This is not always useful, such as with serial devices
	volatile bool connected;
	// flag to indicate whether to program has been asked to stop reading and terminate
	volatile bool interruptRead;
	// flag that allows for waitForData() to be notified when there is new data
    volatile bool cvBool;
    // flag to indicate if this CommConnection is never going to read data from its connection
	bool noReads;
	// flags related to whether the reading thread is running
	bool begun, terminated;
	// flag to indicate whether debugging messages should be displayed
	bool debug;
	// does the reading from the connection by running performReads()
	std::thread *readThread;
	// the mutex that prevents waitForData() from prematurely telling the user there is data
	std::mutex dataMutex;
	// the condition variable that uses the above mutex
	std::condition_variable cv;

	// calls getData(2) and fillBuffer(2)
	// is the function executed by readThread
	// sets cv when there is new data
	void performReads();
	// fills buffer with the data that is in buff and moves the writeIndex forward by bytesRead amount
	void fillBuffer(char *buff, const int &bytesRead);
	// attempts to stop readThread and destroy it
	void closeThread();

	// a child class may attempt to restart the connection with this function as it is called when getData failes
	virtual void failedRead() = 0;
	// the function that the child class implements to do the reading of the data from the connection
	virtual int getData(char *buff, const int &buffSize) = 0;
	// allows for the child to clean up its objects. This is called by terminate()
	virtual void exitGracefully() = 0;
	// allows the child to implement how blocking is done for its connection
	// called by the constructor
    virtual bool setBlocking(const int &blockingTime = -1) = 0;
public:
	CommConnection(const int &blockingTime = -1, const bool &debug = false, const bool &noReads = false);
    CommConnection(const CommConnection &other);
    CommConnection &operator=(const CommConnection &other);

    // starts the readThread
	bool begin();
	// returns how many bytes are available to be read from the buffer immediately
	long long available() const;
	// blocks until there is a byte to be read from the buffer
	int waitForData();
	// returns 1 byte from the buffer if one is available and moves readIndex up by 1
	// if no byte is available, then it returns 0
	char read();
	// fills buff with bytesToRead number of bytes and moves readIndex up by bytesToRead amount
	// buff must be allocated by the caller and is left untouched if no bytes are available to be read
	void read(char *buff, const long long &bytesToRead);
	// fills buff until either buffSize amount of bytes are read, or the character delim is read
	// it will move readIndex up by the number of bytes it put into buff
	// buff must be allocated by the caller
	int readUntil(char *buff, const int &buffSize, const char &delim);
	// returns a string with bytesToRead number of characters if that many bytes can be read
	// if no argument is provided to this function, the string that is returned has all the bytes that are in buffer
	// it will move readIndex up by the number of bytes it put into the string
    std::string readString(const long long &bytesToRead = 0);
    // returns connected
	bool isConnected() const;
	// sets readIndex = writeIndex
	void clearBuffer();	
	// shuts down the connection, and attempts to terminate the read thread
	// calls exitGracefully()
	// called by the destructor
	void terminate();
	// calls write(2)
    bool write(const std::string &buff);

    virtual ~CommConnection();
    // sends the data on the connection
    // buffSize is required to prevent reading past the end of allocated space for buff if the data being sent is not character data
	virtual bool write(const char *buff, const int &buffSize) = 0;
};

#endif //COMMCONNECTION_H
