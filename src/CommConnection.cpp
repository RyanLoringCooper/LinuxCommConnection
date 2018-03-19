/* Copyright 2018 Ryan Cooper (RyanLoringCooper@gmail.com)
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
	//		printReadInformation(bytesRead, buff);
			fillBuffer(buff, bytesRead);
	//		printBuffer(buffer);
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

char *CommConnection::readRange(const int &start, const int &end) {
    char *retval;
    if(end < start) {
        retval = new char[end+_BUFFER_SIZE-start+1];
        memcpy(retval, &buffer[start], _BUFFER_SIZE-start);
        memcpy(&retval[_BUFFER_SIZE-start], &buffer[0], end);
        retval[end+_BUFFER_SIZE-start] = '\0';
    } else {
        retval = new char[end-start+1];
        memcpy(retval, &buffer[start], end-start);
        retval[end-start] = '\0';
    }
    readIndex = end;
    return retval;
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

int CommConnection::waitForDelimitor(const char &delim) {
	int numBytesToRead = 0;
	while(true) {
		int avail = waitForData();
		int i;
		for(i = 0; i < avail; i++) {
			if(buffer[(i+numBytesToRead+readIndex)%_BUFFER_SIZE] == delim) {
				return i+numBytesToRead+1;
			}
		}
		numBytesToRead += i;
	}
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
		int end = readIndex+bytesToRead;
		if(end > _BUFFER_SIZE) {
			end -= _BUFFER_SIZE;
		}
		char *temp = readRange(readIndex, end);
		memcpy(buff, temp, bytesToRead+1);
		delete[] temp;
	}
}

int CommConnection::readUntil(char **buff, const char &delim, const long long &maxBytes, const bool &includeDelim) {
	int i = readIndex, count = 0;
	while(true) {
        count++;
        if(buffer[i] == delim) {
            if(includeDelim) {
                if(i == readIndex) {
                    *buff = new char[2];
                    (*buff)[0] = buffer[readIndex++];
                    (*buff)[1] = '\0';
                    return count;
                } else {
                    i++;
                    break;
                }
            } else {
                if(i == readIndex) {
                    *buff = NULL;
                    return 0;
                }
                count--;
                break;
            }
        } else {
            if((maxBytes > 0 && count == maxBytes) || i == writeIndex-1) {
                break;
            }
            i++;
            if(i == _BUFFER_SIZE) {
                i = 0;
            }
        }
    }
    *buff = readRange(readIndex, i);
    return count;
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
		delete[] buffer;
		exitGracefully();
	}
}

bool CommConnection::write(const std::string &buff) {
	return write(buff.c_str(), buff.length());
} 

CommConnection::~CommConnection() {
	terminate();
}
