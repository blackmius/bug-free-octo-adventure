#include "Pinger.h"

#include <iostream>
#include <arpa/inet.h>
#include <chrono>
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include <numeric>

std::string Pinger::hostnameToIp(const char* host) {
    
    hostent* hostInfo = gethostbyname(host);

    if (hostInfo == nullptr) {
        return std::string("");
    }

    in_addr addr;
    addr.s_addr = *(ulong*)hostInfo->h_addr_list[0];
    
    return std::string(inet_ntoa(addr));
}

Pinger::Pinger(const char* _host) : host(_host) {
    
    ip = hostnameToIp(_host);

    icmpHeader.type = 8;
    icmpHeader.code = 0;
    icmpHeader.checksum = 0xfff7;
    icmpHeader.data = 0;

    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = 0;
    sockAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    protocol = getprotobyname(PROTOCOL_NAME);

    socket = ::socket(AF_INET, SOCK_RAW, protocol->p_proto);

    sendPacketsCount = 0;
    recvPacketsCount = 0;
}

int Pinger::Ping() {
    
    if (ip.empty()) {
        std::cerr << "Invalid host.\n";
        return -1;
    }

    if (socket <= 0) {
        std::cerr << "Socket creation failure.\n";
        return -1;
    }

    printf("PING %s (%s).\n", host.c_str(), ip.c_str());

    for (int i = 1;; i++) {

        sockaddr recvAddr;
        uint recvAddrLen;
        char buffer[BUFSIZE];

        using namespace std::chrono;
        auto startTime = high_resolution_clock::now().time_since_epoch().count();

        int sendResult = sendto(socket, &icmpHeader, sizeof(icmpHeader), 0, (sockaddr*)&sockAddr, sizeof(sockAddr));
        if (sendResult > 0) {
            sendPacketsCount++;
        }

        int recvResult = recvfrom(socket, buffer, sizeof(buffer), 0, &recvAddr, &recvAddrLen);
        if (recvResult > 0) {
            auto endTime = high_resolution_clock::now().time_since_epoch().count();
            double time = double(endTime - startTime) * 1e-6;
            
            recvPacketsCount++;

            times.push_back(time);

            printf("%ld bytes from %s: icmp_seq=%d time=%f ms\n", sizeof(buffer), ip.c_str(), i, time);
        }

        
        sleep(1);
    }
    
    return 0;
}

void Pinger::PrintStatistics() {

    double packetLoss = (sendPacketsCount - recvPacketsCount) / sendPacketsCount * 100;            
    
    double minPingTime = *std::min_element(times.begin(), times.end()).base();
    double maxPingTime = *std::max_element(times.begin(), times.end()).base();
    double avgPingTime = std::reduce(times.begin(), times.end()) / times.size();
    double mdev = getMdev();

    printf("\n--- %s ping statistic ---\n", ip.c_str());
    printf("%d sent, %d received, %.0f%% packet loss\n", sendPacketsCount, recvPacketsCount, packetLoss);
    
    if (recvPacketsCount != 0) {
        printf("rtt min/avg/max/mdev = %f/%f/%f/%f ms\n", minPingTime, avgPingTime, maxPingTime, mdev);
    }
}

double Pinger::getMdev() {
    
    double avgPingTime = std::reduce(times.begin(), times.end()) / times.size();
    
    double mdev = 0;

    for (auto& time : times) {
        mdev += pow(time - avgPingTime, 2.0);
    }

    mdev /= times.size();
    mdev = sqrt(mdev);

    return mdev;
}