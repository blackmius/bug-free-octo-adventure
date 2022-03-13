#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <vector>

#include "icmp_hdr_t.h"

class Pinger {

private:
    static const int BUFSIZE = 64;
    const char* PROTOCOL_NAME = "ICMP";

    std::string host;
    std::string ip;

    int sendPacketsCount;
    int recvPacketsCount;

    icmp_hdr_t icmpHeader;

    int socket;
    sockaddr_in sockAddr;

    std::vector<double> times;

private:
    static std::string hostnameToIp(const char* host);
    double getMdev();    

public:
    Pinger() {}
    Pinger(const char* host);

    int Ping();
    void PrintStatistics();


};