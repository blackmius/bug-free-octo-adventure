// Файл содержащий определение класса, полей и функций, необходимых для работы ping'a.

#pragma once

#include <string> // std::string
#include <netdb.h> // hostent, gethostbyname(), sockaddr_in
#include <memory>

#include "PingLogger.h"
#include "ICMPHeader.h"

// Тип реализующий функционал ping'a.
class Pinger
{

private:
    // Информация о хосте предоставляемая пользователем.
    std::string host;
    std::string ip;
    
    // Частота отправки пакетов (в секундах).
    int64_t timeout = 1;

    // Дескриптор сокета.
    int socket;
    // Информация, используемая при создании сокета.
    sockaddr_in sockAddr;

    // Статистика.
    int sendPacketsCount;
    int recvPacketsCount;

    double minPingTime;
    double maxPingTime;
    double avgPingTime;
    double preAvgPingTime;
    double mdev;

    // Реккурентное стандартное отклонение
    // http://www.scalaformachinelearning.com/2015/10/recursive-mean-and-standard-deviation.html
    double zhmih;

    // Логгер.
    PingLogger *pingLogger;

    /**
     * @brief Функция для расчета чек-суммы пакета.
     * @param buf Указатель на начало пакета.
     * @param size Размер пакета.
     * @return uint16_t Чек-сумма.
     * 
     * Подробнее
     * https://datatracker.ietf.org/doc/html/rfc1071
     * 
     */
    uint16_t calculateChecksum(uint16_t* buf, int32_t size);

    /**
     * @brief Выполняет отправку пакета.  
     * 
     * @param lastPacketSendTime указатель на переменную, хранящую время отправки последнего пакета.
     * @return 0 - отправка не производилась, -1 - ошибка отправки, > 0 - количество отправленных байт.
     */
    int sendPackage(int64_t *lastPacketSendTime);

    /**
     * @brief Выполняет получение пакета.
     * 
     * @param buffer указатель на буфер получаемого пакета.
     * @param bufferSize размер буфера получаемого пакета.
     * @return 0 - сокет пуст, -1 - ошибка получения, >0 - количество полученных байт.
     */
    int recvPackage(unsigned char *buffer, size_t bufferSize);

    /**
     * @brief Обновляет статистику и выводит ифнормацию о последнем полученном пакете.
     * 
     * @param buffer указатель на буфер полученого пакета. 
     * @param bufferSize размер буфера полученного пакета.
     * 
     * @return 0 - Обновление статистики вывод и запись в лог прошли успешно, -1 - При записи в лог произошла ошибка
     */
    int updateStatistic(unsigned char *buffer, size_t bufferSize);

    /**
     * @brief Выводит финальную статистику работы пинга.
     * 
     * @return 0 - Вывод и запись в лог прошли успешно, -1 - При записи в лог произошла ошибка
     */
    int outputStatistic();

public:

    // Определяет, должен ли продолжать работу Pinger.
    bool running = true;

public:

    /**
     * @brief Создает новый экземпляр Pinger
     * 
     * @param host IPv4 или домен хоста.
     * @param ip Строго IPv4 хоста.
     * @param pingLogger Указатель на логгер.
     */
    Pinger(const char* host, const std::string& ip, PingLogger *pingLogger);

    /**
     * @brief Выполняет функционал ping'a.
     * 
     * @return 0 - если ping прошел без ошибок, иначе код ошибки (RECV_ERROR, SEND_ERROR, LOG_WRITE_ERROR).
     */
    int Ping();

    /**
     * @brief Выполняет валидацию переданных аргументов. 
     * 
     * @param argc количество переданных аргументов. 
     * @return 0 - c аргументами все впорядке, иначе код ошибки (INVALID_ARGUMENTS_COUNT).
     */
    static int ValidateArgs(int argc);

    /**
     * @brief Создает объект Pinger.
     * 
     * @param host IPv4 или домен хоста.
     * @param pingLogger Указатель на логгер.
     * @return std::tuple<Pinger*, int> Указатель на объект и 0 если нет ошибок, иначе nullptr и код ошибки (SOCKET_CREATION_ERROR, LOG_WRITE_ERROR).
     */
    static std::tuple<Pinger*, int> CreatePinger(const char* host, const std::string& ip, PingLogger *pingLogger);

    /**
     * @brief Выводит сообщение ошибки и завершает программу.
     * 
     * @param errorCode Код ошибки.
     * @param logger Указатель на логгер.
     */
    static void EndWithError(int errorCode, PingLogger* logger = nullptr);

    /**
     * @brief Определяет IPv4 хоста, если это необходимо.
     * 
     * @param host IPv4 или домен хоста.
     * @return std::tuple<std::string, int> IPv4 хоста и 0 если нет ошибок, иначе пустую строку и код ошибки (HOST_ERROR).
     */
    static std::tuple<std::string, int> GetIp(const char* host);
};