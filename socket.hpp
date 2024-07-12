//
// Created by david on 11/07/24.
//

#ifndef WEBSERVER_SOCKET_HPP
#define WEBSERVER_SOCKET_HPP

#include "ipaddress.hpp"
#include "error.hpp"

#include "unistd.h"

namespace network::tcp
{
  class Socket
  {
  private:
    network::ip::IPv4Address address_;
    unsigned short port_;
    int socket_fd_;
    int new_socket_fd_;

    sockaddr_in socketAddress_;
    socklen_t socketAddressLen_;

    void openSocket();
    void bindSocket();
    void closeSocket();

    void acceptConnection();

  public:
    Socket(const network::ip::IPv4Address& addr, unsigned short port);
    ~Socket();


     void listenSocket();
     void sendResponse() const;

  };

} // network::tcp

#endif //WEBSERVER_SOCKET_HPP
