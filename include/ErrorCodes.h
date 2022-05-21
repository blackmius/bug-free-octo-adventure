// Файл, содержащий пользовательские коды возврата.

#pragma once

const int INVALID_ARGUMENTS_COUNT = 201; // Неверное количество аргументов.
const int CANT_OPEN_OR_CREATE_LOG = 202; // Ошибка при открытии или создании лог-файла.
const int HOST_ERROR = 203;              // Ошибка при определении ip-адреса.
const int SOCKET_CREATION_ERROR = 204;   // Ошибка при создании сокета.
const int SEND_ERROR = 205;              // Ошибка при отправке пакета.
const int RECV_ERROR = 206;              // Ошибка при получении пакета.
const int LOG_WRITE_ERROR = 207;         // Ошибка при записи в лог-файл.