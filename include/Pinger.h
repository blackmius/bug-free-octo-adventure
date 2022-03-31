#include <string>
#include <sys/socket.h>
#include <netdb.h>

#include "ICMPHeader.h"

class Pinger {

private:

    static const int BUFSIZE = 64;

    std::string host;
    std::string ip;

    int sendPacketsCount;
    int recvPacketsCount;

    uint16_t pid;

    int socket;
    sockaddr_in sockAddr;

    double minPingTime;
    double maxPingTime;
    double avgPingTime;
    double preAvgPingTime;
    double mdev;

    uint16_t calculateChecksum(uint16_t* buf, int32_t size);

public:

    bool ShouldEnd;

public:

    explicit Pinger(const char* host);

    void Ping();
};