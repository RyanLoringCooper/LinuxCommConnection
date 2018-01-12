## CommConnection

This library was made to simplify the use of TCP, UDP, and serial devices. It is modeled after the Arduino wire library and provides a similar interface. 

Classes from the library can be initialized like this:
```
SerialConnection con("/dev/ttyUSB0", B57600);
NetworkConnection tcpClient(54321, SOCK_STREAM, "192.168.1.100");
NetworkConnection tcpServer(54321);
NetworkConnection udpClient(12345, SOCK_DGRAM, "10.0.0.100");
NetworkConnection udpServer(12345, SOCK_DGRAM);
```
Then to make use of the objects the code is like this:
```
bool keepReading = true;
con.begin();
while(keepReading) {
    con.waitForData();
    std::string data = con.readString();
    con.write(std::string("Something you would like to send"));
}
```
