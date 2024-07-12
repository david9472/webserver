//
// Created by david on 11/07/24.
//

#ifndef WEBSERVER_TRACE_HPP
#define WEBSERVER_TRACE_HPP

#include "logger.hpp"

#include <fmt/core.h>

#include <utility>

namespace logging
{

  class Trace
  {
    static constexpr unsigned char INDENT_FACTOR{2};

    const std::chrono::high_resolution_clock::time_point enter_time;
    const std::string functionName_;

  public:
    explicit Trace(std::string functionName) : enter_time(std::chrono::high_resolution_clock::now()),
                                               functionName_(std::move(functionName))
    {
      logging::Logger::getInstance().log(logging::LogLevel::TRACE,
                                         fmt::format("ENTERING {}",
                                                     functionName_));
    }

    ~Trace()
    {
      const auto leave_time = std::chrono::high_resolution_clock::now();
      const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(leave_time - enter_time).count();
      logging::Logger::getInstance().log(logging::LogLevel::TRACE,
                                         fmt::format("LEAVING {} ({}µs)",
                                                     functionName_, duration));
    }
  };

}

#endif //WEBSERVER_TRACE_HPP
