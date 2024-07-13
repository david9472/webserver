//
// Created by david on 11/07/24.
//

#ifndef WEBSERVER_SOCKET_HPP
#define WEBSERVER_SOCKET_HPP

#include "ipaddress.hpp"
#include "error.hpp"
#include "socketfiledescriptor.hpp"

#include <functional>
#include <thread>

namespace network::tcp
{
  class Socket
  {
  private:
    class ConnectionManager
    {
    private:
      int accepted_socket_;
      std::thread worker_;

    public:
      explicit ConnectionManager(std::function<void(void)> task, int accepted_socket) : accepted_socket_(accepted_socket)
      {
        worker_ = std::thread(std::move(task));
        logging::Logger::getInstance().log(logging::LogLevel::DEBUG, fmt::format("Started worker thread (TID: {})", logging::formatThreadId(worker_.get_id())));
      }

      ~ConnectionManager()
      {
        worker_.join();
        logging::Logger::getInstance().log(logging::LogLevel::DEBUG, fmt::format("Joined worker thread (TID: {})", logging::formatThreadId(worker_.get_id())));
      }

      ConnectionManager(ConnectionManager&& other)  noexcept : accepted_socket_(other.accepted_socket_), worker_(std::move(other.worker_)) {}

      [[nodiscard]] int getAcceptedSocket() const { return accepted_socket_; }
    };

    std::vector<ConnectionManager> connections_;

    network::ip::IPv4Address address_;
    unsigned short port_;
    SocketFileDescriptor socket_;
    bool shutdown_;
    sockaddr_in socketAddress_{};
    socklen_t socketAddressLen_;

    void openSocket();
    void bindSocket();
    void closeSocket();

    void acceptConnection(SocketFileDescriptor& accepted_socket);
    void handleConnection(SocketFileDescriptor accepted_socket) const;
  public:
    Socket(const network::ip::IPv4Address &addr, unsigned short port);
    ~Socket();

    void listenSocket();
    void shutdownSocket();
  };

} // network::tcp


#endif //WEBSERVER_SOCKET_HPP
