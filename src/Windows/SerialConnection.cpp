/* SerialConnection.cpp -- Implements functions for a class that connects to a serial device
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
#include "../SerialConnection.h"

// protected:
void SerialConnection::failedRead() {
    printf("Failed to read from socket with error: %d", GetLastError());
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
