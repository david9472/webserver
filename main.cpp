#include <iostream>

#include "error.hpp"
#include "socket.hpp"



int main()
{
  logging::Logger::getInstance().setLogLevel(logging::LogLevel::DEBUG);
  network::tcp::Socket socket(network::ip::IPv4Address(127, 0, 0, 1), 8080);

  socket.listenSocket([](const std::string& msg_received)
  {
    return "200 OK";
  });
}
