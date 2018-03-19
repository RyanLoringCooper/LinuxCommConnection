#include <iostream>
#include <cstdio>
#include <string>
#include "../src/CommConnection.h"

class FileConnection : public CommConnection {
    protected:

        void failedRead() {
            std::cerr << "Could not read from file.\n";
        }

        int getData(char *buff, const int &buffSize) {
            if(!_readEOF) {
                int read = fread(buff, sizeof(char), buffSize, fp);
                if(feof(fp)) {
                    _readEOF = true;
                }
                if(read == 0) {
                    buff[0] = '\0';
                }
                //std::cout << "bytes read: " << read << " " << buff << std::endl;
                return read;
            } else {
                buff[0] = '\0';
                return 0;
            }
        }
    
        void exitGracefully() {
            if(fp != NULL) {
                fclose(fp);
                fp = NULL;
            }
        }

        bool setBlocking(const int &blockingTime = -1) {
            return false;
        }
    public:
        FILE *fp;
        bool _readEOF;
        
        FileConnection(const char *filePath, const int &blockingTime = -1, const bool &debug = false, const bool &noReads = false) : CommConnection(blockingTime, debug, noReads) {
            _readEOF = false;
            fp = fopen(filePath, "r+");
            if(!fp) {
                std::cerr << filePath << " not found.\n";
                exit(1);
            }
            connected = true;
        }
        ~FileConnection() {
            exitGracefully();
        }
        
        bool write(const char *buff, const int &buffSize) {
            return fwrite(buff, sizeof(char), buffSize, fp);
        }
        using CommConnection::write;
};

int main(int argc, char *argv[]) {
    if(argc != 2) {
        std::cerr << "Not enough arguments. Provide a path to a file. Usage:\n\t" << argv[0] << " apath\nWhere apath is a valid path to a file to be read and written to.\n";
        return -1;
    }
    FileConnection fc(argv[1]);
    fc.begin();
    fc.write(std::string("Some data is being written to the file with great care\n\t{hey:\"what\"}\nA dog.\n"));
    fseek(fc.fp, 0, SEEK_SET);
    fc.waitForData();
    char *buff;
    int read = fc.readUntil(&buff, '}', 0, true);
    std::cout << "***\nread: " << read << " buff: " << buff << " next: " << fc.read() << std::endl;
    delete[] buff;
    buff = NULL;

    std::cout << "***Testing readUntil with false\n";
    fseek(fc.fp, 0, SEEK_SET);
    fc._readEOF = false;
    fc.waitForData();
    read = fc.readUntil(&buff, '}', 0, false);
    std::cout << "***\nread: " << read << " buff: " << buff << " next:" << fc.read() << std::endl;
    buff = NULL;

    std::cout << "*** Testing wrap around if _BUFFER_SIZE == 128\n";
    fc.write(std::string("So much more data. So much \n"));
    fseek(fc.fp, 0, SEEK_SET);
    fc._readEOF = false;
    int avail = fc.waitForData();
    delete[] buff;
    buff = new char[avail+1];
    fc.read(buff, avail);
    std::cout << "buff: " << buff << std::endl;

    std::cout << "*** TESTING readString and waitForDelimitor\n";
    fc.clearBuffer();
    fseek(fc.fp, 0, SEEK_SET);
    fc._readEOF = false;
    int bytesToRead = fc.waitForDelimitor('}');
    std::string readstr = fc.readString(bytesToRead);
    std::cout << "bytesToRead: " << bytesToRead << "readstr: " << readstr << std::endl;

    std::cout << "*** TESTING and waitForDelimitor\n";
    fc.clearBuffer();
    fseek(fc.fp, 0, SEEK_SET);
    fc._readEOF = false;
    bytesToRead = fc.waitForDelimitor('}');
    delete[] buff;
    buff = new char[bytesToRead+1];
    fc.read(buff, bytesToRead);
    std::cout << "bytesToRead: " << bytesToRead << " strlen(buff): " << strlen(buff) << " buff: " << buff << std::endl;
    delete[] buff;
    return 0;
}
