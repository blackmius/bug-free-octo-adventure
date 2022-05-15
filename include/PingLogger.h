#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <time.h>

// Тип реализующий функционал журнала.
class PingLogger {
private:
  //Лог-файл
  FILE * fd;
public:
  /**
   * @brief Construct a new PingLogger object
   * 
   * @param path Назвагие лог-файла. (Значение по умолчанию - log.txt)
   */
  PingLogger(const char* path = "log.txt");

  //Деструктор объекта Журнала
  ~PingLogger();

  /**
   * @brief Возвращает точное системное время
   * @return std::string - Системное время.
   */
  std::string time_now();

  /**
   * @brief Записывает события в журнал и выводит их в терминале.
   * 
   * @param message Текст события.
   * @param show Параметр, который определяет выводить событие в терминале или только сохранить его в журнале.
   * (Значение true - вывод и в терминале и в журнале, значение false - вывод только в журнале)
   */
  void log_message(std::string message, bool show = false);
};