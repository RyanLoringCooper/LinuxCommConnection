/* Copyright 2018 Ryan Cooper (RyanLoringCooper@gmail.com)
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "../SerialConnection.h"

// protected:
void SerialConnection::failedRead() {
    printf("Failed to read from socket with error: %d", GetLastError());
    connected = false;
}

int SerialConnection::getData(char *buff, const int &buffSize) {
    if(!connected)
        return -1; 
    DWORD toRead, bytesRead, errors;
    COMSTAT status;
    ClearCommError(handler, &errors, &status);
    if (status.cbInQue > 0) {
        if (status.cbInQue > MAX_DATA_LENGTH) {
            toRead = MAX_DATA_LENGTH;
        } else {
            toRead = status.cbInQue;
        }
        if(ReadFile(handler, buff, toRead, &bytesRead, NULL)) {
            return bytesRead;
        } else if(GetLastError() == ERROR_IO_PENDING) {
            return 0;
        } else {
            return -1;
        }
    }  
    return 0;
}

void SerialConnection::exitGracefully() {
    char *stop = new char[4];
    memset(stop, 0, 4);
    write(stop, 4);
    write("reset", 5);
    if (connected) {
        connected = false;
        CloseHandle(handler);
    }
    delete[] stop;
}

// public:
SerialConnection::SerialConnection(const char *portName, const int &blockingTime, const bool &debug, const bool &noReads) : CommConnection(blockingTime, debug, noReads) {
    handler = CreateFileA(static_cast<LPCSTR>(portName),
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (handler == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            printf("Error: Port %s not available\n", portName);
        } else {
            printf("Unknown error.");
        }
    } else {
        DCB dcbSerialParameters = { 0 };

        if (!GetCommState(handler, &dcbSerialParameters)) {
            printf("Failed to get current serial parameters");
        } else {
            dcbSerialParameters.BaudRate = CBR_9600;
            dcbSerialParameters.ByteSize = 8;
            dcbSerialParameters.StopBits = ONESTOPBIT;
            dcbSerialParameters.Parity = NOPARITY;
            dcbSerialParameters.fDtrControl = DTR_CONTROL_ENABLE;

            if (!SetCommState(handler, &dcbSerialParameters)) {
                printf("ALERT: could not set Serial port parameters\n");
            } else {
                connected = true;
                PurgeComm(handler, PURGE_RXCLEAR | PURGE_TXCLEAR);
                Sleep(2000);
            }
        }
    }
}

SerialConnection::SerialConnection(const SerialConnection &other) : CommConnection(other) {
    if(this == &other) {
        return;
    }
    *this = other;
}

SerialConnection &SerialConnection::operator=(const SerialConnection &other) {
    if(this == &other) {
        return *this;
    }
    handler = other.handler;
    CommConnection::operator=(other);
    return *this;
}

bool SerialConnection::write(const char *buff, const int &buffSize) {
    if(!connected) 
        return false;
    DWORD bytesSend, errors;
    COMSTAT status;
    if (!WriteFile(handler, (void*)buff, buffSize, &bytesSend, 0)) {
        ClearCommError(handler, &errors, &status);
        printf("Failed to send: %s", buff);
        return false;
    } else {
        return true;
    }
}
