#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include "PingLogger.h"


#include "ICMPHeader.h"

class Pinger {

private:

    static const int BUFSIZE = 64;

    std::string host;
    std::string ip;

    int64_t timeout = 1;

    int sendPacketsCount;
    int recvPacketsCount;

    int socket;
    sockaddr_in sockAddr;

    double minPingTime;
    double maxPingTime;
    double avgPingTime;
    double preAvgPingTime;
    double mdev;
    PingLogger *pingLogger;
    uint16_t calculateChecksum(uint16_t* buf, int32_t size);

public:
    bool running = true;

public:

    explicit Pinger(const char* host, PingLogger *pingLogger);

    void Ping();
};