//
// Created by david on 11/07/24.
//

#ifndef WEBSERVER_LOGGER_HPP
#define WEBSERVER_LOGGER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <memory>
#include <mutex>
#include <thread>
#include<fmt/core.h>

namespace logging
{
  std::size_t formatThreadId(const std::thread::id& tid);

  enum class LogLevel
  {
    DEBUG,
    TRACE,
    INFO,
    WARNING,
    ERROR,
  };

  class Logger
  {
  public:
    static Logger &getInstance();

    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    void setLogLevel(LogLevel level);
    void setLogThreadId(bool logTID);
    void setOutputStream(std::ostream &os);

    void log(LogLevel level, const std::string &message);
    void log(LogLevel level, const std::string &fileName, const std::string &functionName, const long lineNumber, const std::string& message);

  private:
    Logger();

    static std::string getCurrentTime();
    static std::string formatTime(const std::chrono::system_clock::time_point &time_point);

    static std::string logLevelToString(LogLevel level);

    bool logThreadId_;
    LogLevel loglevel_;
    std::ostream *outputstream_;
    std::mutex outputmutex_;
  };
} // logging

#endif //WEBSERVER_LOGGER_HPP
