#include "Pinger.h"
#include "PingLogger.h"

#include <iostream>
#include <arpa/inet.h>
#include <chrono>
#include <unistd.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <sstream>

using namespace std::chrono;

Pinger::Pinger(const char* _host, PingLogger* _logger) : host(_host) {
    std::stringstream strFormat;

    // Переводим имя хоста в ip. Желательно проверять на то, является ли _host изначально готовым ip, но необязательно.
    pingLogger = _logger;
    pingLogger->log_message("Pinger object initialization");

    const hostent* hostInfo = gethostbyname(_host);
    if (hostInfo == nullptr) {
        throw std::runtime_error("Ivalid host.");
    }
    pingLogger->log_message("Pinger tries to get ip from address");
    in_addr addr;
    addr.s_addr = *(ulong*)hostInfo->h_addr_list[0];

    ip = inet_ntoa(addr);
    strFormat << "Pinger got ip: " << ip.c_str() << "from address";
    pingLogger->log_message(strFormat.str());
    strFormat.str().resize(0);
    timeout *= 10e8;

    pingLogger->log_message("Pinger tries to create socket");
    // Готовим информацию, необходимую для создания сокета.
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = 1025;
    sockAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    // Создаем сокет. Если что не так, выбрасываем исключение.
    socket = ::socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (socket <= 0) {
        perror("");
        throw std::runtime_error("Socket creation failure");
    }
    pingLogger->log_message("Socket was successfully created");

    // Инициализируем переменные статистики.
    sendPacketsCount = 0;
    recvPacketsCount = 0;

    minPingTime = -1.0;
    maxPingTime = -1.0;
    avgPingTime = 0.0;
    preAvgPingTime = 0.0;
    mdev = 0.0;
    pingLogger->log_message("Pinger object initialization complete");

}

void Pinger::Ping() {
    std::stringstream strFormat;
    pingLogger->log_message("Starting ping");

    // Для формулы которую написал Даня (для расчета mdev), но она не очень правильная, на сайте немного по-другому.
    // Я не знаю как ее назвать
    double zhmih = 0;

    // Время отправки последнего пакета.
    int64_t lastPacketSendTime = 0;

    // Время, в течение которого ожидаем ответа.
    struct timeval tv { 0, 10000 };

    // Переменные, которые нужны для получения ответа.
    // 20 байт - ip-пакет + 16 байт - icmp-пакет
    unsigned char buffer[36] = "";

    // Необходимы для получения ответа.
    sockaddr recvAddr;
    uint recvAddrLen;

    strFormat << "PING " << host.c_str() << " (" << ip.c_str() << ").";
    pingLogger->log_message(strFormat.str(), true);
    strFormat.str("");

    // Будет выполняться пока пользователь не нажмет Ctrl + C.
    while (running) {
        // Фиксируем текущее время.
        int64_t now = high_resolution_clock::now().time_since_epoch().count();

        // Если прошло timeout времени с последней отправки пакета, отправляем новый пакет.
        if (now - lastPacketSendTime >= timeout) {
            // Заполняем пакет.
            Packet pckt;
            memset(&pckt, 0, sizeof(pckt));
            pckt.hdr.type = 8;
            pckt.hdr.un.echo.sequence = sendPacketsCount + 1;
            pckt.data = now;
            pckt.hdr.checksum = calculateChecksum((uint16_t*)&pckt, sizeof pckt);

            // Отправляем пакет и проверяем результат.
            int sendResult = sendto(socket, &pckt, sizeof(pckt), 0, (sockaddr*)&sockAddr, sizeof(sockAddr));
            if (sendResult) {
                sendPacketsCount++;
                lastPacketSendTime = now;
            }
        }

        // Получаем ответ и проверяем результат.
        fd_set fd;
        FD_ZERO(&fd);
        FD_SET(socket, &fd);
        int n = select(socket+1, &fd, 0, 0, &tv);
        if (n <= 0) {
            continue;
        }

        int recvResult = recvfrom(socket, buffer, sizeof(buffer), 0, &recvAddr, &recvAddrLen);
        if (recvResult) {
            // Фиксируем время получения пакета.
            now = high_resolution_clock::now().time_since_epoch().count();

            // Преобразуем байты в структуру пакета.
            Packet pckt;
            pckt = *(Packet*)&buffer[20]; // 20 байт - размер ip-пакета
            
            // Считаем, сколько шел пакет.
            double time = double(now - pckt.data) * 1e-6;

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
            strFormat << sizeof(buffer)<<" bytes from "<<ip.c_str()<<": icmp_seq="<<pckt.hdr.un.echo.sequence<<" time="<<time<<" ms";
            pingLogger->log_message(strFormat.str(), true);
            strFormat.str("");
        }
    }
    // Отправка пакетов завершена.

    // Считаем потери пакетов.
    double packetLoss = (sendPacketsCount - recvPacketsCount) / sendPacketsCount * 100;

    // Выводим статистику. 
    strFormat<<"--- "<<ip.c_str()<<" ping statistic ---";
    pingLogger->log_message(strFormat.str(), true);
    strFormat.str("");
    strFormat<<sendPacketsCount<<" sent, "<<recvPacketsCount<<" received, "<<packetLoss<<"% packet loss";
    pingLogger->log_message(strFormat.str(), true);
    strFormat.str("");
    
    // Если мы ничего не получили, то выводить статистику по времени нет смысла.
    if (recvPacketsCount != 0) {
        strFormat<<"rtt min/avg/max/mdev = "<<minPingTime<<"/"<<avgPingTime<<"/"<<maxPingTime<<"/"<<mdev<<" ms";
        pingLogger->log_message(strFormat.str(), true);
        strFormat.str("");

    }
    pingLogger->log_message("Ping is completed");
}

uint16_t Pinger::calculateChecksum(uint16_t *buf, int32_t size) {
    int32_t nleft = size;
    int32_t sum = 0;
    uint16_t *w = buf;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1) {
        sum += *(uint8_t *)w;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    uint32_t checksum = ~sum;

    return checksum;
}