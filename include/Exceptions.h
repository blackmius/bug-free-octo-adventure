#pragma once

#include <stdexcept> // std::runtime_error

// Класс исключения, используемый до создания логгера. 
class BeforeLogError : public std::runtime_error
{
public:

    BeforeLogError(const std::string& message) : std::runtime_error(message) {}

};

// Класс исключения, используемый после создания логгера.
class AfterLogError : public std::runtime_error
{
public:

    AfterLogError(const std::string& message) : std::runtime_error(message) {}

};