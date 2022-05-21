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

Pinger::Pinger(const char* _host, const std::string& _ip, PingLogger* _logger) : host(_host), ip(_ip), pingLogger(_logger)
{
    // Для фомирования отформатированной строки.
    std::stringstream strFormat;

    // Проверка записи данных в лог
    int wroteResult;

    // Записываем в журнал событие о начале создания обьекта 
    wroteResult = pingLogger->log_message("Pinger object initialization");

    if (wroteResult != 0)
    {
        perror("");
        throw LogWriteError();
    }
    
    timeout *= 10e8;

    // Записываем в журнал событие о попытки создать сокет
    wroteResult = pingLogger->log_message("Pinger tries to create socket");

        if (wroteResult != 0)
    {
        perror("");
        throw LogWriteError();
    }
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
    wroteResult = pingLogger->log_message("Socket was successfully created");

    if (wroteResult != 0)
    {
        perror("");
        throw LogWriteError();
    }

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
    wroteResult = pingLogger->log_message("Pinger object initialization complete");

    if (wroteResult != 0)
    {
        perror("");
        throw LogWriteError();
    }

}

int Pinger::Ping()
{
    // Проверка записи данных в лог
    int wroteResult;
    std::stringstream strFormat;
    strFormat << "PING " << host.c_str() << " (" << ip.c_str() << ").";
    wroteResult = pingLogger->log_message(strFormat.str(), true);
    if (wroteResult !=0)
        return LOG_WRITE_ERROR;

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
                switch (recvResult)
                {
                    case 0:
                        break;
                    case -1:
                        return RECV_ERROR;
                    default:
                        int updateResult = updateStatistic(buffer, sizeof(buffer));
                        switch (updateResult)
                        {
                        case 0:
                            break;
                        case -1:
                            return LOG_WRITE_ERROR;
                        }
                }
                break;
        }
    }
    // Отправка пакетов завершена.

    // Выводим статистику.
    int outputResult = outputStatistic();
    switch (outputResult)
    {
    case 0:
        break;
    case -1:
        return LOG_WRITE_ERROR;
    }

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

int Pinger::updateStatistic(unsigned char *buffer, size_t bufferSize)
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
    int wroteResult = pingLogger->log_message(strFormat.str(), true);
    if (wroteResult != 0)
        return -1;

    return 0;
}

int Pinger::outputStatistic()
{
    // Считаем потери пакетов.
    double packetLoss = (sendPacketsCount - recvPacketsCount) / sendPacketsCount * 100;
    // Проверка записи данных в лог
    int wroteResult;
    // Выводим статистику. 
    std::stringstream strFormat;
    strFormat<<"\n--- "<<ip.c_str()<<" ping statistic ---";
    wroteResult = pingLogger->log_message(strFormat.str(), true);
    if (wroteResult != 0)
        return -1;
    strFormat.str("");

    strFormat<<sendPacketsCount<<" sent, "<<recvPacketsCount<<" received, "<<packetLoss<<"% packet loss";
    wroteResult = pingLogger->log_message(strFormat.str(), true);
    if (wroteResult != 0)
        return -1;
    strFormat.str("");
    
    // Если мы ничего не получили, то выводить статистику по времени нет смысла.
    if (recvPacketsCount != 0)
    {
        strFormat<<"rtt min/avg/max/mdev = "<<minPingTime<<"/"<<avgPingTime<<"/"<<maxPingTime<<"/"<<mdev<<" ms";
        wroteResult = pingLogger->log_message(strFormat.str(), true);
        if (wroteResult != 0)
            return -1;
    }
    return 0;
}

std::tuple<Pinger*, int> Pinger::CreatePinger(const char* host, const std::string& ip, PingLogger *pingLogger)
{
    Pinger* pinger;
    try
    {
        pinger = new Pinger(host, ip, pingLogger);
    }
    catch(const SocketCreationError& e)
    {
        return std::tuple(nullptr, SOCKET_CREATION_ERROR);
    }
    catch(const LogWriteError& e)
    {
        return std::tuple(nullptr, LOG_WRITE_ERROR);
    }

    return std::tuple(pinger, 0);
}

void Pinger::EndWithError(int errorCode, PingLogger* logger)
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
        case LOG_WRITE_ERROR:
            errorMessage = "Can't write event in log file.\n";
            break;
    }

    if (logger)
    {
        int wroteResult = logger->log_message(errorMessage, true);
        if (wroteResult != 0)
        {
            std::cerr << errorMessage << '\n';
            EndWithError(LOG_WRITE_ERROR);
        }
    }
    else
    {
        std::cerr << errorMessage << '\n';
    }

    exit(errorCode);
}

std::tuple<std::string, int> Pinger::GetIp(const char* host)
{
    std::stringstream strFormat;

    sockaddr_in s;
    int result = inet_pton(AF_INET, host, &(s.sin_addr));
    if (result != 0)
    {
        return std::tuple(std::string(host), 0);
    }
    else
    {
        // Переводим имя хоста в ip.
        const hostent* hostInfo = gethostbyname(host);
        // Проверяем хост. Если что не так, выбрасываем исключение.
        if (hostInfo == nullptr)
        {
            return std::tuple("", HOST_ERROR);
        }
        // Записываем в журнал событие о попытки утилиты получить ip адрес введенного хоста

        in_addr addr;
        addr.s_addr = *(ulong*)hostInfo->h_addr_list[0];

        return std::tuple(inet_ntoa(addr), 0);
    }
}