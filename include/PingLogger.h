#pragma once

#include <string> // std::string
#include <fstream> // FILE, fopen(), fprintf(), fclose()
#include <time.h> // strftime(), localtime(), tm
#include <iostream> // printf()
#include <tuple> // std::tuple

/**
 * @brief Журнал
 * 
 * Делает записи в указанный при инициализации файл.
 * Для полноты лога весь вывод в stdout лучше делать через журнал,
 * указывая параметр show=true в log_message
 */
class PingLogger
{
private:
  FILE * fd; // дескриптор журнала
public:
  /**
   * @brief Создает новый экземпляр PingLogger
   * 
   * @param path Назвагие лог-файла. (Значение по умолчанию - log.txt)
   */
  PingLogger(const char* path = "log.txt");

  //Деструктор объекта Журнала
  ~PingLogger();

  /**
   * @brief Возвращает точное системное время
   * @return std::string текущее время в формате "<день>.<месяц>.<год> <часы>:<минуты>:<секунды>"
   */
  std::string time_now();

  /**
   * @brief вывод сообщения в журнал
   * 
   * @param message сообщения для вывода в журнал
   * @param show при указании show=true выводить сообщение в stdout
   *             сделано, чтобы все сообщения для пользователя также
   *             попадали в журнал
   */
  void log_message(std::string message, bool show = false);

  static std::tuple<PingLogger*, int> CreateLogger(int argc, char **argv);
};