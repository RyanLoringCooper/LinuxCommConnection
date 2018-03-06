/* NetworkConnection.cpp -- Implements shared functions and includes platform specific code
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

#if defined(__linux__) || defined(__linux) || defined(linux)
    #include "Linux/NetworkConnection.cpp"
#elif defined(_WIN32)
    #include "Windows/NetworkConnection.cpp"
#else
    #error Unsupported os
#endif

NetworkConnection::NetworkConnection(const int &port, const int &connectionType, const char *ipaddr, const int &blockingTime, const bool &debug, const bool &noReads) : CommConnection(blockingTime, debug, noReads) {
	this->connectionType = connectionType;
	if(strcmp(ipaddr, "") == 0) {
        server = true;
		if(!setupServer(port)) {
			fprintf(stderr, "Could not setup socket server on port %d.\n", port);
		}
	} else {
        server = false;
		if(!setupClient(ipaddr, port)) {
			fprintf(stderr, "Could not setup socket client connection to %s:%d", ipaddr, port);
		}
	}
}

NetworkConnection::~NetworkConnection() {
	terminate();
}
