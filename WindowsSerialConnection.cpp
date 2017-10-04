#include "SerialPort.h"

// protected:
void SerialPort::failedRead() {
    printf("Failed to read from socket with error: %d", GetLastError());
}

int SerialPort::getData(char *buff, const int &buffSize) {
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

void SerialPort::exitGracefully() {
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

SerialPort::SerialPort(char *portName, const bool &noReads) : CommConnection(noReads) {
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

SerialPort::~SerialPort() {
    exitGracefully();   
}

bool SerialPort::write(const char *buff, const int &buffSize) {
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
