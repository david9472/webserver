//
// Created by david on 11/07/24.
//

#ifndef WEBSERVER_IPADDRESS_HPP
#define WEBSERVER_IPADDRESS_HPP

#include <arpa/inet.h>

#include <fmt/core.h>

#include <cinttypes>
#include <ostream>


namespace network::ip
{
  struct IPv4Address
  {
    uint8_t network_hi;
    uint8_t network_lo;
    uint8_t host_hi;
    uint8_t host_lo;

    IPv4Address(uint8_t nw_hi, uint8_t nw_lo, uint8_t hst_hi, uint8_t hst_lo) :
        network_hi(nw_hi), network_lo(nw_lo), host_hi(hst_hi), host_lo(hst_lo)
    {}

    [[nodiscard]] std::string to_string() const
    {
      return fmt::format("{}.{}.{}.{}",
                         network_hi,
                         network_lo,
                         host_hi,
                         host_lo);

    }

    explicit operator in_addr_t() const
    {
      return inet_addr(to_string().c_str());
    }
  };

  std::ostream &operator<<(std::ostream &os, const IPv4Address &addr)
  {
    os << addr.to_string();
    return os;
  }
}

#endif //WEBSERVER_IPADDRESS_HPP
