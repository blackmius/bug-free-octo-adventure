#include "Pinger.h"
#include "PingLogger.h"
#include "Exceptions.h"
#include "ErrorCodes.h"

#include <arpa/inet.h> // inet_ntoa()
#include <chrono> // std::chrono::high_resolution_clock
#include <unistd.h> // int64_t, uint16_t, int32_t
#include <cmath> // sqrt()
#include <cstring> // memset()
#include <sstream> // std::stringstream

using namespace std::chrono;

Pinger::Pinger(const char* _host, PingLogger* _logger) : host(_host), pingLogger(_logger)
{
    // Для фомирования отформатированной строки.
    std::stringstream strFormat;

    // Записываем в журнал событие о начале создания обьекта 
    pingLogger->log_message("Pinger object initialization");

    // Переводим имя хоста в ip.
    const hostent* hostInfo = gethostbyname(_host);
    // Проверяем хост. Если что не так, выбрасываем исключение.
    if (hostInfo == nullptr)
    {
        throw HostError();
    }
    // Записываем в журнал событие о попытки утилиты получить ip адрес введенного хоста
    pingLogger->log_message("Pinger tries to get ip from address");

    in_addr addr;
    addr.s_addr = *(ulong*)hostInfo->h_addr_list[0];

    ip = inet_ntoa(addr);
    // Записываем в журнал событие об успешном получении ip адреса
    strFormat << "Pinger got ip: " << ip.c_str() << "from address";
    pingLogger->log_message(strFormat.str());
    strFormat.str().resize(0); //Очищаем форматированную строку
    
    timeout *= 10e8;

    // Записываем в журнал событие о попытки создать сокет
    pingLogger->log_message("Pinger tries to create socket");
    // Готовим информацию, необходимую для создания сокета.
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = 1025;
    sockAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    // Создаем сокет. Если что не так, выбрасываем исключение.
    socket = ::socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (socket <= 0)
    {
        perror("");
        throw SocketCreationError();
    }
    // Записываем в журнал событие об успешном создании сокета
    pingLogger->log_message("Socket was successfully created");

    // Инициализируем переменные статистики.
    sendPacketsCount = 0;
    recvPacketsCount = 0;

    minPingTime = -1.0;
    maxPingTime = -1.0;
    avgPingTime = 0.0;
    preAvgPingTime = 0.0;
    mdev = 0.0;
    zhmih = 0.0;

    // Записываем в журнал событие об успешной инициализации объекта Pinger
    pingLogger->log_message("Pinger object initialization complete");

}

int Pinger::Ping()
{
    // Записываем в журнал событие о начале пинга
    pingLogger->log_message("Starting ping");

    std::stringstream strFormat;
    strFormat << "PING " << host.c_str() << " (" << ip.c_str() << ").";
    pingLogger->log_message(strFormat.str(), true);
    strFormat.str("");

    // Время отправки последнего пакета.
    int64_t lastPacketSendTime = 0;

    // Будет выполняться пока пользователь не нажмет Ctrl + C.
    while (running)
    {
        // Пробуем отправить пакет.
        int sendResult = sendPackage(&lastPacketSendTime);
        switch (sendResult) {
            case -1:
                return SEND_ERROR;
            default:
                // Буфер в который записывается ответ.
                // 20 байт - ip-пакет + 16 байт - icmp-пакет.
                unsigned char buffer[36] = "";

                int recvResult = recvPackage(buffer, sizeof(buffer));
                switch (recvResult) {
                    case 0:
                        break;
                    case -1:
                        return RECV_ERROR;
                    default:
                        updateStatistic(buffer, sizeof(buffer));
                }
                break;
        }
    }
    // Отправка пакетов завершена.

    // Выводим статистику.
    outputStatistic();

    // Записываем в журнал событие об успешном завершении работы утилиты
    pingLogger->log_message("Ping is completed");

    return 0;
}

uint16_t Pinger::calculateChecksum(uint16_t *buf, int32_t size)
{
    int32_t nleft = size;
    int32_t sum = 0;
    uint16_t *w = buf;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        sum += *(uint8_t *)w;
    }

    // у нас размер буфера всегда 36 байт это значит 0xFFFF * 18 максимум 1 переполнение
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);

    uint32_t checksum = ~sum;

    return checksum;
}

int Pinger::ValidateArgs(int argc)
{
    // Два или три, потому что (1)./ping (2)www.google.com (3)logfile.log.
    if (argc != 2 && argc != 3)
    {
        return INVALID_ARGUMENTS_COUNT;
    }

    return 0;
}

int Pinger::sendPackage(int64_t *lastPacketSendTime) {
    // Фиксируем текущее время.
    int64_t now = high_resolution_clock::now().time_since_epoch().count();
    
    if (now - *lastPacketSendTime >= timeout)
    {
        // Заполняем пакет.
        Packet pckt;
        memset(&pckt, 0, sizeof(pckt));
        pckt.hdr.type = 8;
        pckt.hdr.un.echo.sequence = sendPacketsCount + 1;
        pckt.data = now;
        pckt.hdr.checksum = calculateChecksum((uint16_t*)&pckt, sizeof pckt);

        // Отправляем пакет и проверяем результат.
        int sendResult = sendto(socket, &pckt, sizeof(pckt), 0, (sockaddr*)&sockAddr, sizeof(sockAddr));
        if (sendResult)
        {
            sendPacketsCount++;
            *lastPacketSendTime = now;
        }
        return sendResult;
    }

    return 0;
}

int Pinger::recvPackage(unsigned char *buffer, size_t bufferSize)
{
    // Время, в течение которого ожидаем ответа.
    struct timeval tv { 0, 10000 };

    // Ждем пока в сокете что-нибудь появится.
    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(socket, &fd);
    int n = select(socket+1, &fd, 0, 0, &tv);
    if (n <= 0)
    {
        return 0;
    }

    // Необходимы для получения ответа.
    sockaddr recvAddr;
    uint recvAddrLen;

    int recvResult = recvfrom(socket, buffer, bufferSize, 0, &recvAddr, &recvAddrLen);
    
    return recvResult;
}

void Pinger::updateStatistic(unsigned char *buffer, size_t bufferSize)
{
    // Фиксируем время получения пакета.
    int64_t now = high_resolution_clock::now().time_since_epoch().count();

    // Преобразуем байты в структуру пакета.
    Packet pckt;
    pckt = *(Packet*)&buffer[20]; // 20 байт - размер ip-пакета
    
    // Считаем, сколько шел пакет.
    double time = double(now - pckt.data) * 1e-6;

    recvPacketsCount++;

    // Ищем наименьшее время.
    if (time < minPingTime || minPingTime == -1)
    {
        minPingTime = time;
    }
    
    // Ищем наибольшее время
    if (time > maxPingTime)
    {
        maxPingTime = time;
    }

    // Считаем среднее время пинга на данный момент.
    preAvgPingTime = avgPingTime;
    avgPingTime = (1 - 1.0 / recvPacketsCount) * avgPingTime + time / recvPacketsCount;
    
    // Считаем среднее отклонение на данный момент.
    zhmih = zhmih + (time - avgPingTime) * (time - preAvgPingTime); 
    mdev = sqrt(zhmih / recvPacketsCount);

    // Выводим информацию о последнем пинге.
    std::stringstream strFormat;
    strFormat << bufferSize <<" bytes from "<<ip.c_str()<<": icmp_seq="<<pckt.hdr.un.echo.sequence<<" time="<<time<<" ms";
    pingLogger->log_message(strFormat.str(), true);
}

void Pinger::outputStatistic()
{
    // Считаем потери пакетов.
    double packetLoss = (sendPacketsCount - recvPacketsCount) / sendPacketsCount * 100;

    // Выводим статистику. 
    std::stringstream strFormat;
    strFormat<<"\n--- "<<ip.c_str()<<" ping statistic ---";
    pingLogger->log_message(strFormat.str(), true);
    strFormat.str("");

    strFormat<<sendPacketsCount<<" sent, "<<recvPacketsCount<<" received, "<<packetLoss<<"% packet loss";
    pingLogger->log_message(strFormat.str(), true);
    strFormat.str("");
    
    // Если мы ничего не получили, то выводить статистику по времени нет смысла.
    if (recvPacketsCount != 0)
    {
        strFormat<<"rtt min/avg/max/mdev = "<<minPingTime<<"/"<<avgPingTime<<"/"<<maxPingTime<<"/"<<mdev<<" ms";
        pingLogger->log_message(strFormat.str(), true);
    }
}

std::tuple<Pinger*, int> Pinger::CreatePinger(const char* host, PingLogger *pingLogger)
{
    Pinger* pinger;
    try
    {
        pinger = new Pinger(host, pingLogger);
    }
    catch(const HostError& e)
    {
        return std::tuple(nullptr, HOST_ERROR);
    }
    catch(const SocketCreationError& e)
    {
        return std::tuple(nullptr, SOCKET_CREATION_ERROR);
    }

    return std::tuple(pinger, 0);
}

void Pinger::Diagnostic(int errorCode, PingLogger* logger)
{
    std::string errorMessage;
    switch (errorCode)
    {
        case INVALID_ARGUMENTS_COUNT:
            errorMessage = "Invalid arguments count given. Example: sudo ./ping www.google.com log.log(optional).\n";
            break;
        case CANT_OPEN_OR_CREATE_LOG:
            errorMessage = "Can't open or create log file.\n";
            break;
        case HOST_ERROR:
            errorMessage = "Host detection error.";
            break;
        case SOCKET_CREATION_ERROR:
            errorMessage = "Socket creation failure.";
            break;
        case SEND_ERROR:
            errorMessage = "Error when receiving packet.";
            break;
        case RECV_ERROR:
            errorMessage = "Error when sending packet.";
            break;
    }

    if (logger)
    {
        logger->log_message(errorMessage, true);
    }
    else
    {
        std::cerr << errorMessage << '\n';
    }

    exit(errorCode);
}