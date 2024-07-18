//
// Created by david on 11/07/24.
//

#ifndef WEBSERVER_SOCKET_HPP
#define WEBSERVER_SOCKET_HPP

#include "ipaddress.hpp"
#include "error.hpp"
#include "trace.hpp"
#include "socketfiledescriptor.hpp"
#include "messagequeue.hpp"

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
      explicit ConnectionManager(int accepted_socket) : accepted_socket_(accepted_socket) {}

      ~ConnectionManager()
      {
        const logging::Trace trace(__func__, fmt::format("worker thread TID: {}", logging::formatThreadId(worker_.get_id())));
        if (!worker_.joinable())
        {
          logging::Logger::getInstance().log(logging::LogLevel::WARNING, fmt::format("worker thread not joinable! TID: {}", logging::formatThreadId(worker_.get_id())));
          return;
        }

        worker_.join();
      }

      ConnectionManager(ConnectionManager&& other)  noexcept : accepted_socket_(other.accepted_socket_), worker_(std::move(other.worker_)) {}

      [[nodiscard]] int getAcceptedSocket() const { return accepted_socket_; }

      void start(std::function<void(void)> task)
      {
        worker_ = std::thread(std::move(task));
        logging::Logger::getInstance().log(logging::LogLevel::DEBUG, fmt::format("Started worker thread (TID: {})", logging::formatThreadId(worker_.get_id())));
      }
    };

    std::vector<ConnectionManager> connections_;

    network::ip::IPv4Address address_;
    unsigned short port_;
    SocketFileDescriptor socket_;
    std::mutex shutdown_mutex_;
    bool shutdown_;
    sockaddr_in socketAddress_{};
    socklen_t socketAddressLen_;
    std::thread listen_socket_thread_;
    std::thread answer_thread_;

    container::message_queue::Queue& message_queue_;

    void openSocket();
    void bindSocket();
    void closeSocket();

    [[nodiscard]] bool isShutdownOngoing(const std::lock_guard<std::mutex>& lock) const;

    void acceptConnection(SocketFileDescriptor& accepted_socket);
    void handleConnection(SocketFileDescriptor accepted_socket);
    void sendResponseThreaded();
    void listenSocketThreaded();
  public:
    Socket(const network::ip::IPv4Address &addr, unsigned short port,  container::message_queue::Queue& message_queue);
    ~Socket();

    void listenSocket();
    void shutdownSocket();
  };

} // network::tcp


#endif //WEBSERVER_SOCKET_HPP
