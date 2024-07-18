//
// Created by david on 13/07/24.
//
#include "serializable.hpp"

std::ostream& operator<<(std::ostream& os, const Serializable& s)
{
  os << s.serialize();
  return os;
}