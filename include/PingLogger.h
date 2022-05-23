// Файл содержащий определение класса, полей и функций, необходимых для работы логгера.

#pragma once

#include <string>   // std::string
#include <fstream>  // FILE, fopen(), fprintf(), fclose()
#include <time.h>   // strftime(), localtime(), tm
#include <iostream> // printf()
#include <tuple>    // std::tuple

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
  FILE *fd; // дескриптор журнала
public:
  /**
   * @brief Создает новый экземпляр PingLogger
   *
   * @param path Назвагие лог-файла. (Значение по умолчанию - log.txt)
   */
  PingLogger(const char *path = "log.txt");

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
   * @return 0 - запись прошла успешно, иначе -1.
   *
   */
  int log_message(std::string message, bool show = false);

  /**
   * @brief Создает новый экземпляр PingLogger
   *
   * @param argc количество аргументов.
   * @param argv сами аргументы.
   * @return std::tuple<PingLogger*, int> указатель на логгер и 0 если нет ошибок, иначе nullptr и код ошибки (CANT_OPEN_OR_CREATE_LOG).
   */
  static std::tuple<PingLogger *, int> CreateLogger(int argc, char **argv);
};