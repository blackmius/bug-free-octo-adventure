#pragma once

#include <stdexcept> // std::runtime_error

// Класс исключения, используемый до создания логгера. 
class SocketCreationError : public std::runtime_error
{
public:

    SocketCreationError(const std::string& message) : std::runtime_error(message) {}

};

// Класс исключения, используемый после создания логгера.
class HostError : public std::runtime_error
{
public:

    HostError(const std::string& message) : std::runtime_error(message) {}

};