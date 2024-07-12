//
// Created by david on 12/07/24.
//

#ifndef WEBSERVER_ERROR_HPP
#define WEBSERVER_ERROR_HPP

#include "logger.hpp"

#include<fmt/core.h>

#include <cerrno>

#include <stdexcept>

namespace logging
{
  class Error : public std::exception
  {
  private:
    std::string error_;
    std::string full_error_;
  public:
    Error(const std::string &fileName, const std::string &functionName, const long lineNumber,
          const std::string &errorMsg) : error_(errorMsg)
    {
      full_error_ = fmt::format("{} in File: {} Function: {} Line: {}", errorMsg, fileName, functionName, lineNumber);
      Logger::getInstance().log(LogLevel::ERROR, full_error_);
    }

    [[nodiscard]] const char *what() const noexcept override
    {
      return error_.c_str();
    }
  };

  class SystemError : public std::exception
  {
    std::string error_;
    std::string full_error_;
  public:
    SystemError(const std::string &fileName, const std::string &functionName, const long lineNumber,
          const std::string &errorMsg) : error_(errorMsg)
    {
      full_error_ = fmt::format("{} (Error Nr: {}, {}) in File: {} Function: {} Line: {}", errorMsg, errno, strerror(errno), fileName, functionName, lineNumber);
      Logger::getInstance().log(LogLevel::ERROR, full_error_);
    }

    [[nodiscard]] const char *what() const noexcept override
    {
      return error_.c_str();
    }
  };


  #define LOC __FILE__, __func__, __LINE__
}

#endif //WEBSERVER_ERROR_HPP
