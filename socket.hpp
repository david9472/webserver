//
// Created by david on 11/07/24.
//

#ifndef WEBSERVER_SOCKET_HPP
#define WEBSERVER_SOCKET_HPP

#include "ipaddress.hpp"
#include "error.hpp"
#include "socketfiledescriptor.hpp"


namespace network::tcp
{
  class Socket
  {
  private:
    network::ip::IPv4Address address_;
    unsigned short port_;
    SocketFileDescriptor socket_;

    sockaddr_in socketAddress_{};
    socklen_t socketAddressLen_;

    void openSocket();

    void bindSocket();

    void closeSocket();

    void acceptConnection(SocketFileDescriptor& accepted_socket);

  public:
    Socket(const network::ip::IPv4Address &addr, unsigned short port);

    ~Socket();


    void listenSocket();

    void sendResponse(const SocketFileDescriptor& accepted_socket) const;

  };

} // network::tcp


#endif //WEBSERVER_SOCKET_HPP
