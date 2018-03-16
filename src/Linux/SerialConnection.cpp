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
	fprintf(stderr, "Failed to read from serial port with error: %d\n", errno);
}

int SerialConnection::getData(char *buff, const int &buffSize) {
	if(!connected)
		return -1; 
	return ::read(ser, buff, buffSize);
}

void SerialConnection::exitGracefully() {
	if (connected && ser > 0) {
		connected = false;
		close(ser);
	}
}

// taken from https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
int SerialConnection::set_interface_attribs (const int &speed, const int &parity) {
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (ser, &tty) != 0) {
		fprintf(stderr, "error %d from tcgetattr\n", errno);
		return -1;
	}

	cfsetospeed (&tty, speed);
	cfsetispeed (&tty, speed);

	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
	// disable IGNBRK for mismatched speed tests; otherwise receive break
	// as \000 chars
	tty.c_iflag &= ~IGNBRK;         // disable break processing
	tty.c_lflag = 0;                // no signaling chars, no echo,
	                                // no canonical processing
	tty.c_oflag = 0;                // no remapping, no delays
	tty.c_cc[VMIN]  = 0;            // read doesn't block
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

	tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
	                                // enable reading
	tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;

	if (tcsetattr (ser, TCSANOW, &tty) != 0) {
		fprintf(stderr, "error %d from tcsetattr\n", errno);
		return -1;
	}
	return 0;
}

// adapted from https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
bool SerialConnection::setBlocking(const int &blockingTime) {
	if(blockingTime == -1) {
        struct termios tty;
        memset (&tty, 0, sizeof(tty));
        if (tcgetattr (ser, &tty) != 0) {
            fprintf(stderr,"error %d from tggetattr\n", errno);
            return false;
        }

        tty.c_cc[VMIN]  = 1;            // sets blocking to true
        tty.c_cc[VTIME] = blockingTime/100;            // 0.5 seconds read timeout

        if (tcsetattr (ser, TCSANOW, &tty) != 0) {
            fprintf(stderr, "error %d setting term attributes\n", errno);
            return false;
        }
    }
	return true;
}


// public:
// speed should have a B in front of it (ie. B57600)
// I recommend parity 0
SerialConnection::SerialConnection(const char *portName, const int &speed, const int &parity, const int &blockingTime, const bool &debug, const bool &noReads) : CommConnection(blockingTime, debug, noReads) {
	connected = false;
	ser = open (portName, O_RDWR | O_NOCTTY | O_SYNC);
	if (ser < 0) {
		fprintf(stderr, "error %d opening %s\n", errno, portName);
		return;
	}
	if(set_interface_attribs (speed, parity) != 0) {
		fprintf(stderr, "Could not set serial parameters.\n");
		return;
	}
	connected = true;
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
    ser = other.ser;
    CommConnection::operator=(other);
    return *this;
}

// Sets errno when it returns false, indicating there was an error with writting
bool SerialConnection::write(const char *buff, const int &buffSize) {
	if(!connected) 
		return false;
	return ::write(ser, buff, buffSize) != -1;
}
