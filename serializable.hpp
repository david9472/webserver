//
// Created by david on 11/07/24.
//

#ifndef WEBSERVER_SERIALIZABLE_HPP
#define WEBSERVER_SERIALIZABLE_HPP

#include <ostream>

class Serializable
{
public:
  [[nodiscard]] virtual std::string serialize() const = 0;

  virtual ~Serializable() = default;
};

std::ostream& operator<<(std::ostream& os, const Serializable& s);
#endif //WEBSERVER_SERIALIZABLE_HPP
