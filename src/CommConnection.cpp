/* CommConnection.cpp -- Implements functions for base class 
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

#include "CommConnection.h"
#include <cstdio>

void printReadInformation(const int &bytesRead, const char *buff) {
	fprintf(stderr, "bytesRead:%d ", bytesRead);
	for(int i = 0 ; i < bytesRead; i++) {
		fprintf(stderr, "%c", buff[i]);
	}
	fprintf(stderr, "\n");
}

void printBuffer(const char *buffer) {
	fprintf(stderr, "buffer:");
	for(int i = 0; i < _BUFFER_SIZE; i++) {
		if(buffer[i] < 32 || buffer[i] > 126) {
			fprintf(stderr, "%02x", buffer[i]);
		} else {
			fprintf(stderr, "%c", buffer[i]);
		}
	}
	fprintf(stderr, "\n");
}

void CommConnection::performReads() {
	char buff[_MAX_DATA_LENGTH];
	memset(buff, 0, _MAX_DATA_LENGTH);
	int bytesRead;
	while(!interruptRead) {
		bytesRead = getData(buff, _MAX_DATA_LENGTH);
		if (bytesRead > 0) {
			printReadInformation(bytesRead, buff);
			fillBuffer(buff, bytesRead);
			printBuffer(buffer);
			cvBool = true;
			try {
				cv.notify_one();
			} catch (...) {
				fprintf(stderr, "cv.notify_one() mutex lock failed\n");
			}
		} else if(bytesRead < 0 && blockingTime < 0) {
			failedRead();
		} else if(blockingTime > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(blockingTime));
		}
	}
}

void CommConnection::fillBuffer(char *buff, const int &bytesRead) {
	int newWriteIndex = bytesRead+writeIndex;
	if(newWriteIndex < _BUFFER_SIZE) {
		memcpy(&buffer[writeIndex], buff, bytesRead);
		writeIndex = newWriteIndex;
	} else {
		fprintf(stderr, "Handling wrap around\n");
		int overflow = newWriteIndex-_BUFFER_SIZE;
		int underflow = _BUFFER_SIZE-writeIndex;
		memcpy(&buffer[writeIndex], buff, underflow);
		memcpy(buffer, &buff[underflow], overflow);
		writeIndex = overflow;
	}
}

void CommConnection::closeThread() {
	interruptRead = true;
	if(readThread != NULL && readThread->joinable()) {
		readThread->join();
		delete readThread;
		readThread = NULL;
	}
}

CommConnection::CommConnection(const int &blockingTime, const bool &debug, const bool &noReads) {
	this->blockingTime = blockingTime;
	this->debug = debug;
	this->noReads = noReads;
	connected = false;
	interruptRead = false;
	terminated = false;
	cvBool = false;
	buffer = new char[_BUFFER_SIZE+1];
	memset(buffer, 0, _BUFFER_SIZE+1);
	readIndex = 0;
	writeIndex = 0;	
}

CommConnection::CommConnection(const CommConnection &other) {
	if(this == &other) {
		return;
	}
	*this = other;
}

CommConnection &CommConnection::operator=(const CommConnection &other) {
	if(this == &other) {
		return *this;
	}
	buffer = new char[_BUFFER_SIZE+1];
	memcpy(buffer, other.buffer, _BUFFER_SIZE+1);
	readIndex = other.readIndex;
	writeIndex = other.writeIndex;
	blockingTime = other.blockingTime;
	connected = other.connected;
	interruptRead = other.interruptRead;
	noReads = other.noReads;
	begun = other.begun;
	terminated = other.terminated;
	cvBool = other.cvBool;
	debug = other.debug;
	if(begun && connected) {
		begin();
	}
	return *this;
}

bool CommConnection::begin() {
	if(connected) {
		begun = true;
		if(!noReads) {
			readThread = new std::thread(&CommConnection::performReads, this);
		}
		return true;
	} else {
		return false;
	}
}

long long CommConnection::available() const {
	long long retval = writeIndex-readIndex;
	if(retval < 0) 
		retval = writeIndex+_BUFFER_SIZE-readIndex;
	return retval;
}

int CommConnection::waitForData() {
	std::unique_lock<std::mutex> lk(dataMutex);
	cv.wait(lk, [this]{
			if(this->cvBool) {
			this->cvBool = false;
			return true;
			} else {
			return false;
			}
			});
	return available();
}

char CommConnection::read() {
	if(available() > 0) {
		if (readIndex + 1 < _BUFFER_SIZE) {
			return buffer[readIndex++];
		} else {
			char retval = buffer[readIndex];
			readIndex = 0;
			return retval;
		}
	} else {
		return 0;
	}
}

void CommConnection::read(char *buff, const long long &bytesToRead) {
	if(bytesToRead <= available()) {
		int newReadIndex = readIndex+bytesToRead;
		if(newReadIndex < _BUFFER_SIZE) {
			memcpy(buff, &buffer[readIndex], bytesToRead);
			readIndex = newReadIndex;
		} else {
			int overflow = newReadIndex-_BUFFER_SIZE;
			int underflow = _BUFFER_SIZE-readIndex;
			memcpy(buff, &buffer[readIndex], underflow);
			memcpy(&buff[underflow], buffer, overflow);
			readIndex = overflow;
		}
	}
}

// does not put the delim character in the buff 
int CommConnection::readUntil(char *buff, const int &buffSize, const char &delim) { // TODO this needs work! Can't handle wrap around case
	int i = readIndex, count = 0, leftOff = 0;
	while(true) {
		if(available() > 0) {
			if(buffer[i] == delim) {
				memcpy(&buff[leftOff], &buffer[readIndex], i-readIndex);
				readIndex = i+1;
				return count;
			} else if(i == _BUFFER_SIZE) {
				memcpy(&buff[leftOff], &buffer[readIndex], i-readIndex);
				leftOff = i-readIndex;
				readIndex = 0;
				i = 0;
			} else {
				i++;
			}
			count++;
			if(count == buffSize) {
				return count;
			}
		}
	}
}

std::string CommConnection::readString(const long long &bytesToRead) {
	long long goingToRead = bytesToRead;
	if(goingToRead == 0) {
		goingToRead = available();
	}
	if(goingToRead <= available()) {
		char buff[goingToRead+1];
		memset(buff, 0, goingToRead+1);
		read(buff, goingToRead);
		return std::string(buff);
	} else {
		return std::string("");
	}
}

bool CommConnection::isConnected() const {
	return connected;
}

void CommConnection::clearBuffer() {
	readIndex = writeIndex;
}

void CommConnection::terminate() {
	if(!terminated) {
		terminated = true;
		cvBool = true;
		cv.notify_all();
		closeThread();
		//delete[] buffer;
		exitGracefully();
	}
}

bool CommConnection::write(const std::string &buff) {
	return write(buff.c_str(), buff.length());
} 

CommConnection::~CommConnection() {
	terminate();
}
