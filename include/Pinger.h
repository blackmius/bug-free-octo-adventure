#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <vector>

#include "ICMPHeader.h"

class Pinger {

private:

    static const int BUFSIZE = 64;

    std::string host;
    std::string ip;

    int sendPacketsCount;
    int recvPacketsCount;

    ICMPHeader icmpHeader;

    int socket;
    sockaddr_in sockAddr;

    double minPingTime;
    double maxPingTime;
    double avgPingTime;
    double preAvgPingTime;
    double mdev;

public:

    bool ShouldEnd;

public:

    Pinger(const char* host);

    void Ping();
};