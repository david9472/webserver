//
// Created by david on 11/07/24.
//

#include "socket.hpp"
#include "trace.hpp"


namespace network::tcp
{
  /// @class Socket
  /// @name Socket
  /// @brief constructor
  /// @param[in] addr : IPv4 Address on which the socket should communicate
  /// @param[in] port : Port on which the socket should communicate
  /// @throws logging::SystemError
  Socket::Socket(const network::ip::IPv4Address &addr, const unsigned short port, MessageQueue& message_queue) : address_(addr), port_(port),
                                                                                                                  socketAddressLen_(sizeof(socketAddress_)),
                                                                                                                  shutdown_(false),
                                                                                                                  message_queue_(message_queue)
  {
    const logging::Trace trace(__func__);
    openSocket();
    bindSocket();
  }

  /// @class Socket
  /// @name ~Socket
  /// @brief destructor which closes potentially open sockets
  /// @throws None
  Socket::~Socket()
  {
    const logging::Trace trace(__func__);
    shutdownSocket();

    if (!listen_socket_thread_.joinable())
    {
      logging::Logger::getInstance().log(logging::LogLevel::WARNING, "Socket listening thread not joinable!");
    }
    else
    {
      listen_socket_thread_.join();
    }

    if (!answer_thread_.joinable())
    {
      logging::Logger::getInstance().log(logging::LogLevel::WARNING, "Socket answer thread not joinable!");
    }
    else
    {
      answer_thread_.join();
    }
  }

  /// @class Socket
  /// @name openSocket
  /// @brief Opens a new socket
  /// @throws logging::SystemError
  void Socket::openSocket()
  {
    const logging::Trace trace(__func__);

    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ < 0)
    {
      throw logging::SystemError(LOC, "Creating a socket failed");
    }
  }

  /// @class Socket
  /// @name closeSocket
  /// @brief Closes the socket
  /// @throws None
  void Socket::closeSocket()
  {
    const logging::Trace trace(__func__);
    socket_ = -1;
  }

  /// @class Socket
  /// @name acceptConnection
  /// @brief Accepts incoming connections
  /// @param[out] accepted_socket : file descriptor of the accepted connection
  /// @throws logging::SystemError
  void Socket::acceptConnection(SocketFileDescriptor &accepted_socket)
  {
    const logging::Trace trace(__func__);
    accepted_socket = accept(socket_, reinterpret_cast<sockaddr *>(&socketAddress_), &socketAddressLen_);
    {
      std::lock_guard<std::mutex> g_shutdown_lock(shutdown_mutex_);
      if (isShutdownOngoing(g_shutdown_lock))
      {
        logging::Logger::getInstance().log(logging::LogLevel::DEBUG, LOC, "Shutdown signal received");
        return;
      }
    }
    if (accepted_socket < 0)
    {
      throw logging::SystemError(LOC,
                                 fmt::format("Failed to accept incoming connection from {}:{}", address_.to_string(),
                                             port_));
    }
  }

  /// @class Socket
  /// @name bindSocket
  /// @brief uses the syscall 'bind' to bind a socket
  /// @throws logging::SystemError
  void Socket::bindSocket()
  {
    const logging::Trace trace(__func__);

    memset(&socketAddress_, 0, sizeof(socketAddress_));
    socketAddress_.sin_family = AF_INET;
    socketAddress_.sin_port = htons(port_);
    socketAddress_.sin_addr.s_addr = static_cast<in_addr_t>(address_);

    if (bind(socket_, reinterpret_cast<sockaddr *>(&socketAddress_), socketAddressLen_))
    {
      throw logging::SystemError(LOC, "Cannot connect socket to address");
    }
  }

  /// @class Socket
  /// @name listenSocket
  /// @brief Accepts incoming connections, starts a new thread for each accepted socket and
  ///        handles the subsequent communications
  /// @throws logging::SystemError
  void Socket::listenSocket()
  {
    const logging::Trace trace(__func__);

    listen_socket_thread_ = std::thread([this](){listenSocketThreaded();});
    answer_thread_ = std::thread([this](){sendResponseThreaded();});
  }

  /// @class Socket
  /// @name listenSocketThreaded
  /// @brief Tasks executed by thread to listen for incoming connections and starting a listener thread
  void Socket::listenSocketThreaded()
  {
    constexpr short MAX_NUMBER_LISTENING_THREADS{5};
    while (true)
    {
      const int result = listen(socket_, MAX_NUMBER_LISTENING_THREADS);
      {
        std::lock_guard<std::mutex> g_shutdown_lock(shutdown_mutex_);
        if (isShutdownOngoing(g_shutdown_lock))
        {
          logging::Logger::getInstance().log(logging::LogLevel::DEBUG, LOC, "Shutdown signal received");
          return;
        }
      }
      if (result < 0)
      {
        throw logging::SystemError(LOC, "Socket listen failed!");
      }

      SocketFileDescriptor accepted_socket;
      acceptConnection(accepted_socket);
      {
        std::lock_guard<std::mutex> g_shutdown_lock(shutdown_mutex_);
        if (isShutdownOngoing(g_shutdown_lock))
          return;
      }

      auto& it = connections_.emplace_back(accepted_socket);
      it.start([this, &accepted_socket]() { handleConnection(std::move(accepted_socket)); });
    }
  }

  /// @class Socket
  /// @name handleConnection
  /// @brief Gets executed by multiple threads to handle multiple accepted sockets at the same time
  /// @param[in] accepted_socket : accepted socket for the communication
  /// @throws logging::SystemError
  void Socket::handleConnection(SocketFileDescriptor accepted_socket)
  {
    const logging::Trace trace(__func__);
    while (true)
    {
      constexpr int SIZE_BUFFER{2048};
      char buffer[SIZE_BUFFER]{};
      const int bytes_received = read(accepted_socket, buffer, SIZE_BUFFER);
      {
        std::lock_guard<std::mutex> g_shutdown_lock(shutdown_mutex_);
        if (isShutdownOngoing(g_shutdown_lock))
        {
          logging::Logger::getInstance().log(logging::LogLevel::DEBUG, LOC, "Shutdown signal received");
          return;
        }
      }

      if (bytes_received < 0)
      {
        throw logging::SystemError(LOC, fmt::format("Read failed! fd: {}", accepted_socket.operator int()));
      }

      logging::Logger::getInstance().log(logging::LogLevel::INFO, LOC, fmt::format("Message received: {}", buffer));

      message_queue_.enqueueReceivedMessage({buffer, accepted_socket});

      std::optional<Message> response{message_queue_.retrieveResponseMessageNonBlocking()};
      if (response.has_value())
      {
        const int bytes_sent = send(response.value().getSocket(), response.value().getMessageString().c_str(), response.value().getMessageString().length(), MSG_NOSIGNAL);
        if (bytes_sent != response.value().getMessageString().length())
        {
          logging::Logger::getInstance().log(logging::LogLevel::INFO, LOC, "Send failed! Stopping listening thread");
          return;
        }
        else
          logging::Logger::getInstance().log(logging::LogLevel::DEBUG, LOC, fmt::format("Send successful! Msg: {}", response.value().getMessageString()));
      }
    }
  }

  /// @class Socket
  /// @name sendResponseThreaded
  /// @brief task executed by a thread to retrieve messages from the respond queue and sending the via the provided socket
  /// @throws None
  void Socket::sendResponseThreaded()
  {
    while (true)
    {
      Message response{message_queue_.retrieveResponseMessage()};
      {
        std::lock_guard<std::mutex> g_shutdown_lock(shutdown_mutex_);
        if (isShutdownOngoing(g_shutdown_lock))
        {
          logging::Logger::getInstance().log(logging::LogLevel::DEBUG, LOC, "Shutdown signal received");
          return;
        }
      }
      const int bytes_sent = send(response.getSocket(), response.getMessageString().c_str(), response.getMessageString().length(), MSG_NOSIGNAL);
      if (bytes_sent != response.getMessageString().length())
      {
        logging::Logger::getInstance().log(logging::LogLevel::INFO, LOC, "Send failed! Stopping listening thread");
        return;
      }
      else
        logging::Logger::getInstance().log(logging::LogLevel::DEBUG, LOC, fmt::format("Send successful! Msg: {}", response.getMessageString()));
    }
  }

  /// @class Socket
  /// @name shutdownSocket
  /// @brief End the communication on a socket by closing all file descriptors and joining all threads
  void Socket::shutdownSocket()
  {
    const logging::Trace trace(__func__);
    logging::Logger::getInstance().log(logging::LogLevel::DEBUG,
                                       LOC,
                                       fmt::format("Number of thread contexts: {}", connections_.size()));
    shutdown_mutex_.lock();
    shutdown_ = true;
    shutdown_mutex_.unlock();

    socket_ = -1;

    message_queue_.shutdown();

    std::for_each(connections_.begin(), connections_.end(),
                  [](const ConnectionManager &connection_manager) {
                    shutdown(connection_manager.getAcceptedSocket(), SHUT_RDWR);
                  });
  }

  bool Socket::isShutdownOngoing(const std::lock_guard<std::mutex>&) const
  {
    return shutdown_;
  }
};