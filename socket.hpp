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

    void openSocket()
    {
      logging::Logger::getInstance().log(logging::LogLevel::INFO, "Opening socket");

      socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
      if (socket_fd_ < 0)
        throw logging::Error(LOC, "Creating a socket failed");
    }

    void closeSocket()
    {
      logging::Logger::getInstance().log(logging::LogLevel::INFO, "Closing socket");
      close(socket_fd_);
      close(new_socket_fd_);
      socket_fd_ = -1;
    }

    void acceptConnection()
    {
      new_socket_fd_ = accept(socket_fd_, reinterpret_cast<sockaddr*>(&socketAddress_), &socketAddressLen_);
      if (new_socket_fd_ < 0)
        throw logging::Error(LOC, fmt::format("Failed to accept incoming connection from {}:{}", address_.to_string(), port_));
    }

  public:
    Socket(const network::ip::IPv4Address& addr, const unsigned short port) :
        address_(addr), port_(port), socketAddressLen_(sizeof(socketAddress_))
           {
              openSocket();
           }

     ~Socket()
     {
       closeSocket();
     }

     void bindSocket()
     {
       logging::Logger::getInstance().log(logging::LogLevel::INFO, "Binding socket");

       memset(&socketAddress_, 0, sizeof(socketAddress_));
       socketAddress_.sin_family = AF_INET;
       socketAddress_.sin_port = htons(port_);
       socketAddress_.sin_addr.s_addr = static_cast<in_addr_t>(address_);

       if (bind(socket_fd_, reinterpret_cast<sockaddr*>(&socketAddress_), socketAddressLen_))
         throw logging::Error(LOC, "Cannot connect socket to address");
     }

     void listenSocket()
     {
       constexpr short MAX_NUMBER_LISTENING_THREADS{5};
       if (listen(socket_fd_, MAX_NUMBER_LISTENING_THREADS) < 0)
         throw logging::Error(LOC, "Socket listen failed!");

       logging::Logger::getInstance().log(logging::LogLevel::INFO, fmt::format("Listening on {}:{}", address_.to_string(), port_));

       while (true)
       {
         acceptConnection();

         constexpr size_t BUFFER_SIZE{2048};
         std::array<char, BUFFER_SIZE> buffer{};
         const int bytes_received = read(new_socket_fd_, buffer.data(), BUFFER_SIZE);
         if (bytes_received < 0)
           throw logging::Error(LOC, "Failed to read bytes from socket");

         logging::Logger::getInstance().log(logging::LogLevel::INFO, fmt::format("Received {} bytes", bytes_received));
         logging::Logger::getInstance().log(logging::LogLevel::DEBUG, fmt::format("Msg: {}", buffer.data()));

         sendResponse();

         close(new_socket_fd_);
       }
     }


     void sendResponse()
     {
      const std::string responseString{"OK"};
      const long bytesSent = write(new_socket_fd_, responseString.c_str(), responseString.size());
      if (bytesSent == responseString.size())
        logging::Logger::getInstance().log(logging::LogLevel::DEBUG, "Send response successful");
      else
        throw logging::Error(LOC, "Sending response message failed!");
     }
  };

} // network::tcp

#endif //WEBSERVER_SOCKET_HPP
