#include <unistd.h>
#include <sys/wait.h>
#include <thread>
#include <iostream>
#include <chrono>
#include "../src/NetworkConnection.h"

#define PORT 12334
#define BUFF_SIZE 256

const std::string message("Hello"), endingMessage("exit");

void connectionLoop(NetworkConnection *con, const std::string &conIdentifier) {
    char buff[BUFF_SIZE];
    int avail;
    for(int i = 0; i < 10; i++) {
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
    con->terminate();
}

void server() {
    NetworkConnection con = NetworkConnection(PORT, SOCK_DGRAM, "", 10);
    con.begin();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    connectionLoop(&con, "server");
}

void client() {
    NetworkConnection con = NetworkConnection(PORT, SOCK_DGRAM, "127.0.0.1", 10);
    con.begin();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    connectionLoop(&con, "client");
}

int main(int argc, char *argv[]) {
    std::thread t0(server);
    std::thread t1(client);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    t0.join();
    t1.join();
/*
    if(fork() == 0) {
        if(fork() == 0) {
            client();
        } else {
            server();
            int status;
            waitpid(-1, &status, 0);
        }
    } else {
        int status;
        waitpid(-1, &status, 0);
    }
*/
    return 0;
}
