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

namespace logging
{
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
    // Get the singleton instance of the Logger
    static Logger &getInstance()
    {
      static Logger instance;
      return instance;
    }

    // Delete copy constructor and assignment operator
    Logger(const Logger &) = delete;

    Logger &operator=(const Logger &) = delete;

    // Set the logging level
    void setLogLevel(LogLevel level)
    {
      logLevel = level;
    }

    // Set the output stream
    void setOutputStream(std::ostream &os)
    {
      std::lock_guard<std::mutex> guard(outputMutex);
      outputStream = &os;
    }

    // Log a message with a specified log level
    void log(LogLevel level, const std::string &message)
    {
      if (level < logLevel)
      {
        return;
      }

      std::lock_guard<std::mutex> guard(outputMutex);
      *outputStream << getCurrentTime() << " [" << logLevelToString(level) << "] " << message << std::endl;
    }

  private:
    Logger() : logLevel(LogLevel::INFO), outputStream(&std::cout)
    {}

    // Helper to get current time in HH:MM:SS:MS format
    static std::string getCurrentTime()
    {
      const auto now = std::chrono::system_clock::now();
      return formatTime(now);
    }

    static std::string formatTime(const std::chrono::system_clock::time_point &time_point)
    {
      // Convert to time_t to extract time components
      const auto time_t_point = std::chrono::system_clock::to_time_t(time_point);
      const auto tm = *std::localtime(&time_t_point);

      // Get milliseconds
      const auto duration = time_point.time_since_epoch();
      const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration) % 1000;

      // Format the time
      std::ostringstream oss;
      oss << std::put_time(&tm, "%H:%M:%S");
      oss << '.' << std::setw(3) << std::setfill('0') << milliseconds.count();

      return oss.str();
    }

    // Helper to convert log level to string
    static std::string logLevelToString(LogLevel level)
    {
      switch (level)
      {
        case LogLevel::INFO: return "INFO";
        case LogLevel::TRACE: return "TRACE";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::DEBUG: return "DEBUG";
        default: return "UNKNOWN";
      }
    }

    LogLevel logLevel;
    std::ostream *outputStream;
    std::mutex outputMutex;
  };
} // logging

#endif //WEBSERVER_LOGGER_HPP
