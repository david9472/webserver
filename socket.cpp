//
// Created by david on 11/07/24.
//

#include "socket.hpp"

namespace network::tcp
{
    void Socket::openSocket()
    {
      logging::Logger::getInstance().log(logging::LogLevel::INFO, "Opening socket");

      socket_ = socket(AF_INET, SOCK_STREAM, 0);
      if (socket_ < 0)
        throw logging::SystemError(LOC, "Creating a socket failed");
    }

    void Socket::closeSocket()
    {
      logging::Logger::getInstance().log(logging::LogLevel::INFO, "Closing socket");
    }

    void Socket::acceptConnection(SocketFileDescriptor& accepted_socket)
    {
      accepted_socket = accept(socket_, reinterpret_cast<sockaddr*>(&socketAddress_), &socketAddressLen_);
      if (accepted_socket < 0)
        throw logging::SystemError(LOC, fmt::format("Failed to accept incoming connection from {}:{}", address_.to_string(), port_));
    }

    Socket::Socket(const network::ip::IPv4Address& addr, const unsigned short port) : address_(addr), port_(port), socketAddressLen_(sizeof(socketAddress_))
    {
      openSocket();
      bindSocket();
    }

     Socket::~Socket()
     {
       closeSocket();
     }

     void Socket::bindSocket()
     {
       logging::Logger::getInstance().log(logging::LogLevel::INFO, "Binding socket");

       memset(&socketAddress_, 0, sizeof(socketAddress_));
       socketAddress_.sin_family = AF_INET;
       socketAddress_.sin_port = htons(port_);
       socketAddress_.sin_addr.s_addr = static_cast<in_addr_t>(address_);

       if (bind(socket_, reinterpret_cast<sockaddr*>(&socketAddress_), socketAddressLen_))
         throw logging::SystemError(LOC, "Cannot connect socket to address");
     }

     void Socket::listenSocket(const std::function<std::string(const std::string& msg_received)>& response_callback)
     {
       constexpr short MAX_NUMBER_LISTENING_THREADS{5};
       if (listen(socket_, MAX_NUMBER_LISTENING_THREADS) < 0)
         throw logging::SystemError(LOC, "Socket listen failed!");

       logging::Logger::getInstance().log(logging::LogLevel::INFO, fmt::format("Listening on {}:{}", address_.to_string(), port_));

       while (true)
       {
         SocketFileDescriptor accepted_socket;
         acceptConnection(accepted_socket);

         constexpr size_t BUFFER_SIZE{2048};
         std::array<char, BUFFER_SIZE> buffer{};
         const int bytes_received = read(accepted_socket, buffer.data(), BUFFER_SIZE);
         if (bytes_received < 0)
           throw logging::SystemError(LOC, "Failed to read bytes from socket");

         logging::Logger::getInstance().log(logging::LogLevel::INFO, fmt::format("Received {} bytes", bytes_received));
         logging::Logger::getInstance().log(logging::LogLevel::DEBUG, fmt::format("Msg: {}", buffer.data()));

         sendResponse(accepted_socket, response_callback(buffer.data()));
       }
     }

     void Socket::sendResponse(const SocketFileDescriptor& accepted_socket, const std::string& response_string) const
     {
      const long bytesSent = write(accepted_socket, response_string.c_str(), response_string.size());
      if (bytesSent == response_string.size())
        logging::Logger::getInstance().log(logging::LogLevel::DEBUG, "Send response successful");
      else
        throw logging::SystemError(LOC, "Sending response message failed!");
     }
};