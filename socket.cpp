//
// Created by david on 11/07/24.
//

#include "socket.hpp"
#include "trace.hpp"



namespace network::tcp
{
  Socket::Socket(const network::ip::IPv4Address& addr, const unsigned short port) : address_(addr), port_(port), socketAddressLen_(sizeof(socketAddress_)), shutdown_(false)
  {
    const logging::Trace trace(__func__ );
    openSocket();
    bindSocket();
  }

  Socket::~Socket()
  {
    const logging::Trace trace(__func__ );
    closeSocket();
  }

  void Socket::openSocket()
  {
    const logging::Trace trace(__func__ );

    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0)
      throw logging::SystemError(LOC, "Creating a socket failed");
  }

  void Socket::closeSocket()
  {
    const logging::Trace trace(__func__ );
    socket_ = -1;
  }

  void Socket::acceptConnection(SocketFileDescriptor& accepted_socket)
  {
    const logging::Trace trace(__func__ );
    accepted_socket = accept(socket_, reinterpret_cast<sockaddr*>(&socketAddress_), &socketAddressLen_);
    if (shutdown_)
    {
      logging::Logger::getInstance().log(logging::LogLevel::DEBUG, "Shutdown signal received");
      return;
    }
    if (accepted_socket < 0)
      throw logging::SystemError(LOC, fmt::format("Failed to accept incoming connection from {}:{}", address_.to_string(), port_));
  }



     void Socket::bindSocket()
     {
       const logging::Trace trace(__func__ );

       memset(&socketAddress_, 0, sizeof(socketAddress_));
       socketAddress_.sin_family = AF_INET;
       socketAddress_.sin_port = htons(port_);
       socketAddress_.sin_addr.s_addr = static_cast<in_addr_t>(address_);

       if (bind(socket_, reinterpret_cast<sockaddr*>(&socketAddress_), socketAddressLen_))
         throw logging::SystemError(LOC, "Cannot connect socket to address");
     }

     void Socket::listenSocket()
     {
       const logging::Trace trace(__func__ );
       constexpr short MAX_NUMBER_LISTENING_THREADS{5};

       while (true)
       {
         const int result = listen(socket_, MAX_NUMBER_LISTENING_THREADS);
         if (shutdown_)
         {
           logging::Logger::getInstance().log(logging::LogLevel::DEBUG, "Shutdown signal received");
           return;
         }
         else if (result < 0)
           throw logging::SystemError(LOC, "Socket listen failed!");

         SocketFileDescriptor accepted_socket;
         acceptConnection(accepted_socket);

         connections_.emplace_back([this, &accepted_socket](){ handleConnection(std::move(accepted_socket));}, accepted_socket);
       }

     }

     void Socket::handleConnection(SocketFileDescriptor accepted_socket)
     {
       const logging::Trace trace(__func__ );
       while (true)
       {
         constexpr int SIZE_BUFFER{2048};
         char buffer[SIZE_BUFFER]{};
         const int bytes_received = read(accepted_socket, buffer, SIZE_BUFFER);
         if (shutdown_)
         {
           logging::Logger::getInstance().log(logging::LogLevel::DEBUG, "Shutdown signal received");
           return;
         }
         else if (bytes_received < 0)
           throw logging::SystemError(LOC, "Read failed");

         logging::Logger::getInstance().log(logging::LogLevel::INFO, fmt::format("Message received: {}", buffer));

         const std::string response{R"(HTTP/1.1 200 OK\r\n\n)"};
         const int bytes_sent = send(accepted_socket, response.c_str(), response.size(), 0);
         if (bytes_sent != response.size())
           throw logging::SystemError(LOC, "send failed");
       }
     }

     void Socket::shutdownSocket()
     {
        const logging::Trace trace(__func__ );
       logging::Logger::getInstance().log(logging::LogLevel::DEBUG, fmt::format("Number of thread contexts: {}", connections_.size()));
        shutdown_ = true;
        socket_ = -1;

        std::for_each(connections_.begin(), connections_.end(),
                      [](const ConnectionManager& connection_manager)
                      {
                        shutdown(connection_manager.getAcceptedSocket(), SHUT_RDWR);
                      });

       logging::Logger::getInstance().log(logging::LogLevel::DEBUG, "Joined all threads");
     }


};