#include <thread>
#include <chrono>
#include "../NetworkConnection.h"
#define PORT 12334

NetworkConnection *con0, *con1;
char *exitbuf = "exit";

void connectionLoop(NetworkConnection *con, const char *sendbuf, const int &sendbuflen) {
    char buff[2048];
    int avail;
    for(int i = 0; i < 10; i++) {
        con->write(sendbuf, sendbuflen);
        avail = con->waitForData();
        con->read(buff, avail);
        printf("%s\n", buff);
        if(strcmp(buff, exitbuf) == 0) {
            break; 
        }
        memset(buff, 0, avail);
    }
    con->write(exitbuf, 5);
    con->terminate();
}

void startCon0() {
    char buff[2048];
    con0 = new NetworkConnection(PORT, SOCK_DGRAM, "", 100);
    printf("fack!\n");
    con0->begin();
    printf("fack!\n");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    connectionLoop(con0, "derp", 5);
}

void startCon1() {
    char buff[2048];
    con1 = new NetworkConnection(PORT, SOCK_DGRAM, "127.0.0.1", 100);
    printf("fack!\n");
    con1->begin();
    printf("fack!\n");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    connectionLoop(con1, "dddd", 5);
}

int main(int argc, char *argv[]) {
    std::thread t0(startCon0);
    std::thread t1(startCon1);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    t0.join();
    t1.join();
    printf("wack!\n");
    delete con0;
    printf("wack!\n");
    delete con1;
    printf("wack!\n");
    return 0;
}
