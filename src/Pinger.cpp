#include "Pinger.h"

#include <iostream>
#include <arpa/inet.h>
#include <chrono>
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <exception>

Pinger::Pinger(const char* _host) : host(_host) {

    // Переводим имя хоста в ip. Желательно проверят на то, является ли _host изначально готовым ip.
    const hostent* hostInfo = gethostbyname(_host);

    if (hostInfo == nullptr) {
        throw std::runtime_error("Ivalid host.");
    }

    in_addr addr;
    addr.s_addr = *(ulong*)hostInfo->h_addr_list[0];

    ip = inet_ntoa(addr);

    // Настраиваем заголовок (https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol#:~:text=8%20%E2%80%93%20Echo%20Request,used%20to%20ping)
    icmpHeader.type = 8;
    icmpHeader.code = 0;
    icmpHeader.checksum = 0xfff7;
    icmpHeader.data = 0;

    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = 0;
    sockAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    socket = ::socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (socket <= 0) {
        throw std::runtime_error("Socket creation failure");
    }

    sendPacketsCount = 0;
    recvPacketsCount = 0;

    minPingTime = -1.0;
    maxPingTime = -1.0;
    avgPingTime = 0.0;
    preAvgPingTime = 0.0;
    mdev = 0.0;

    ShouldEnd = false;
}

void Pinger::Ping() {

    // Для формулы которую написал Даня, но она не очень правильная, на сайте немного по-другому.
    // Я не знаю как ее назвать
    double zhmih = 0;
    
    printf("PING %s (%s).\n", host.c_str(), ip.c_str());

    // Будет выполняться пока пользователь не нажмет Ctrl + C.
    for (int i = 1; !ShouldEnd; i++) {
        
        // Переменные, которые нужны для получения ответа.  
        sockaddr recvAddr;
        uint recvAddrLen;
        char buffer[BUFSIZE];

        // Фиксируем начальное время.
        using namespace std::chrono;
        auto startTime = high_resolution_clock::now().time_since_epoch().count();

        // Отправляем пакет и проверяем результат.
        int sendResult = sendto(socket, &icmpHeader, sizeof(icmpHeader), 0, (sockaddr*)&sockAddr, sizeof(sockAddr));
        if (sendResult <= 0) {
            continue;
        }
        sendPacketsCount++;

        // Получаем ответ и проверяем результат.
        int recvResult = recvfrom(socket, buffer, sizeof(buffer), 0, &recvAddr, &recvAddrLen);
        if (recvResult > 0) {

            // Если все отлично - фиксируем время и вычисляем разность (время прошедшее с отправки). 
            auto endTime = high_resolution_clock::now().time_since_epoch().count();
            double time = double(endTime - startTime) * 1e-6;
            
            recvPacketsCount++;

            // Ищем наименьшее время.
            if (time < minPingTime || minPingTime == -1) {
                minPingTime = time;
            }
            
            // Ищем наибольшее время
            if (time > maxPingTime) {
                maxPingTime = time;
            }

            // Считаем среднее время пинга на данный момент.
            preAvgPingTime = avgPingTime;
            avgPingTime = (1 - 1.0 / recvPacketsCount) * avgPingTime + time / recvPacketsCount;
            
            // Считаем среднее отклонение на данный момент.
            zhmih = zhmih + (time - avgPingTime) * (time - preAvgPingTime); 
            mdev = sqrt(zhmih / recvPacketsCount);

            // Выводим информацию о последнем пинге.
            printf("%ld bytes from %s: icmp_seq=%d time=%f ms\n", sizeof(buffer), ip.c_str(), i, time);
        }

        sleep(1);
    }
    
    // Считаем потери пакетов.
    double packetLoss = (sendPacketsCount - recvPacketsCount) / sendPacketsCount * 100;            

    // Выводим статистику. 
    printf("\n--- %s ping statistic ---\n", ip.c_str());
    printf("%d sent, %d received, %.0f%% packet loss\n", sendPacketsCount, recvPacketsCount, packetLoss);
    
    // Если мы ничего не получили, то выводить статистику по времени нет смысла.
    if (recvPacketsCount != 0) {
        printf("rtt min/avg/max/mdev = %f/%f/%f/%f ms\n", minPingTime, avgPingTime, maxPingTime, mdev);
    }
}