//
// Created by david on 13/07/24.
//

#ifndef WEBSERVER_COLOR_HPP
#define WEBSERVER_COLOR_HPP

#include <ostream>

namespace logging
{
  namespace color
  {
    enum class Code
    {
      FG_RED = 31,
      FG_GREEN = 32,
      FG_YELLOW = 33,
      FG_BLUE = 34,
      FG_MAGENTA = 35,
      FG_CYAN = 36,
      FG_DEFAULT = 39,
      BG_RED = 41,
      BG_GREEN = 42,
      BG_BLUE = 44,
      BG_DEFAULT = 49
    };

    class Modifier
    {
      Code code_;
    public:
      constexpr Modifier(Code code) : code_(code)
      {}

      [[nodiscard]] std::string to_string() const
      {
        return "\033[" + std::to_string(static_cast<short>(code_)) + "m";
      }

      friend std::ostream &operator<<(std::ostream &os, const Modifier &mod)
      {
        return os << mod.to_string();
      }
    };

    constexpr Modifier DEFAULT_COLOR(Code::FG_DEFAULT);
  }
} // logging

#endif //WEBSERVER_COLOR_HPP
