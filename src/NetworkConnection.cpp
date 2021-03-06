/* Copyright 2018 Ryan Cooper (RyanLoringCooper@gmail.com)
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
    setBlocking(blockingTime);
}

NetworkConnection::~NetworkConnection() {
	terminate();
}
