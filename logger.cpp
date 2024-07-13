#include "logger.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <thread>
#include<fmt/core.h>

namespace logging
{
  /// @name formatThreadId
  /// @brief converts a std::thread::id to a size_t
  /// @param[in] tid : thread-id
  /// @throws None
  std::size_t formatThreadId(const std::thread::id &tid)
  {
    return std::hash<std::thread::id>{}(tid);
  }

  /// @class Logger
  /// @name Logger::Logger
  /// @brief constructor
  /// @param[in,out] None
  /// @throws None
  Logger::Logger() : loglevel_(LogLevel::INFO), outputstream_(&std::cout), logThreadId_(false)
  {}

  /// @class Logger
  /// @name getInstance
  /// @brief returns an instance of a Logger object
  /// @param[in,out] None
  /// @throws None
  Logger& Logger::getInstance()
  {
    static Logger instance;
    return instance;
  }

  /// @class Logger
  /// @name setLogLevel
  /// @brief Sets the log level
  /// @param[in] level : new log level
  /// @throws None
  void Logger::setLogLevel(LogLevel level)
  {
    loglevel_ = level;
  }

  /// @class Logger
  /// @name setLogThreadId
  /// @brief Controls if the thread-id should be logged
  /// @param[in] logTID : true = log TID; false = do not log TID
  /// @throws None
  void Logger::setLogThreadId(bool logTID)
  {
    logThreadId_ = logTID;
  }

  /// @name setOutputStream
  /// @brief sets the output stream where the log statements should be written
  /// @param[in] os : new output stream
  /// @throws None
  void Logger::setOutputStream(std::ostream &os)
  {
    std::lock_guard<std::mutex> guard(outputmutex_);
    outputstream_ = &os;
  }

  /// @class Logger
  /// @name log
  /// @brief log a message with a specified log level
  /// @param[in] level : log level of the message
  /// @param[in] message : log message
  /// @throws None
  void Logger::log(LogLevel level, const std::string &message)
  {
    if (level < loglevel_)
    {
      return;
    }

    const std::string logEntry{fmt::format("{} [{}]{} {}", getCurrentTime(), logLevelToString(level),
                                           (logThreadId_ ? "[" +
                                                           std::to_string(formatThreadId(std::this_thread::get_id())) +
                                                           "]" : ""), message)};
    std::lock_guard<std::mutex> guard(outputmutex_);
    *outputstream_ << logEntry << std::endl;
  }

  /// @class Logger
  /// @name getCurrentTime
  /// @brief Helper to get current time in HH:MM:SS:MS format
  /// @param[in,out] None
  /// @throws None
  std::string Logger::getCurrentTime()
  {
    const auto now = std::chrono::system_clock::now();
    return formatTime(now);
  }

  /// @class Logger
  /// @name formatTime
  /// @brief Helper to format time the given time to HH:MM:SS:MS format
  /// @param[in] time_point : time point to format
  /// @throws None
  std::string Logger::formatTime(const std::chrono::system_clock::time_point &time_point)
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
  /// @class Logger
  /// @name logLevelToString
  /// @brief converts a LogLevel to its string representation
  /// @param[in] level : LogLevel to convert
  /// @throws None
  std::string Logger::logLevelToString(LogLevel level)
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
} // logging
