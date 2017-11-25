#include <iostream>
#include <sys/types.h> 
#include <sys/socket.h>
#include <thread>
#include <chrono>
#include <string>
#include "../src/NetworkConnection.h"

#define BUFF_SIZE 256

const std::string message("Hello"), endingMessage("exit");

void connectionLoop(NetworkConnection *con, const std::string &conIdentifier) {
    char buff[BUFF_SIZE];
    int avail;
    for(int i = 0; i < 5; i++) {
        con->write(message);
        std::cout << conIdentifier << " sent: " << message << std::endl;
        avail = con->waitForData();
        con->read(buff, avail);
        std::cout << conIdentifier << " recv: " << buff << std::endl;
        if(strcmp(buff, endingMessage.c_str()) == 0) {
            break;
        }
        memset(buff, 0, avail);
    }
    con->write(endingMessage);
    con->write(endingMessage);
    con->terminate();
}

void connection(const int &port, const int &connectionType, const char *ipaddr, const int &delayTime, const std::string &conIdentifier) {
    NetworkConnection con = NetworkConnection(port, connectionType, ipaddr, delayTime);
    con.begin();
    connectionLoop(&con, conIdentifier);
}

int main(int argc, char *argv[]) {
    int connectionTypes[] = {SOCK_STREAM, SOCK_DGRAM};
    int delayTimes[] = {100, 10, 1, 0, -1};
    int port = 12345;
    for(int i = 0; i < 5; i++) {
        for(int j = 0; j < 2; j++) {
            std::cout << "Testing "
                << (connectionTypes[j] == SOCK_DGRAM ? "UDP" : "TCP")
                << " with delay " << delayTimes[i] << std::endl;
            std::thread server(connection, port, connectionTypes[j], "", delayTimes[i], std::string("server"));
            std::thread client(connection, port, connectionTypes[j], "127.0.0.1", delayTimes[i], std::string("client"));
            server.join();
            client.join();
            std::cout << "Test complete\n\n";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            port++;
        }
    }
}
