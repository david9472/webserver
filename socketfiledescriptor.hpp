//
// Created by david on 12/07/24.
//

#ifndef WEBSERVER_SOCKETFILEDESCRIPTOR_HPP
#define WEBSERVER_SOCKETFILEDESCRIPTOR_HPP

#include "logger.hpp"
#include "error.hpp"

#include "unistd.h"

namespace network
{

  class SocketFileDescriptor
  {
  private:
    int socket_fd_{-1};

    void closeSocketFd()
    {
      if (socket_fd_ < 0)
        return;

      logging::Logger::getInstance().log(logging::LogLevel::DEBUG, "Closing socket file descriptor");
      close(socket_fd_);
      socket_fd_ = -1;
    }

  public:
    SocketFileDescriptor() = default;

    SocketFileDescriptor(int socket) : socket_fd_(socket)
    {
      if (socket_fd_ < 0)
      {
        throw logging::Error(LOC, "Invalid value for file descriptor");
      }
    }

    // Delete copy constructor and assignment operator to prevent copying
    SocketFileDescriptor(const SocketFileDescriptor &) = delete;

    SocketFileDescriptor &operator=(const SocketFileDescriptor &) = delete;

    // Move constructor
    SocketFileDescriptor(SocketFileDescriptor &&other) noexcept: socket_fd_(other.socket_fd_)
    {
      other.socket_fd_ = -1; // Invalidate the moved-from object
    }

    // Move assignment operator
    SocketFileDescriptor &operator=(SocketFileDescriptor &&other) noexcept
    {
      if (this != &other)
      {
        // Close the current file descriptor
        if (socket_fd_ >= 0)
        {
          closeSocketFd();
        }
        // Transfer ownership of the file descriptor
        socket_fd_ = other.socket_fd_;
        other.socket_fd_ = -1; // Invalidate the moved-from object
      }
      return *this;
    }

    SocketFileDescriptor &operator=(int fd)
    {
      if (socket_fd_ != fd)
      {
        // Close the current file descriptor if valid
        if (socket_fd_ >= 0)
        {
          closeSocketFd();
        }
        // Assign the new file descriptor
        socket_fd_ = fd;
      }
      return *this;
    }

    ~SocketFileDescriptor()
    {
      closeSocketFd();
    }

    operator int() const
    {
      return socket_fd_;
    }
  };

}
#endif //WEBSERVER_SOCKETFILEDESCRIPTOR_HPP
