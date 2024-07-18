#include <iostream>

#include "error.hpp"
#include "trace.hpp"
#include "socket.hpp"
#include "messagequeue.hpp"

#include <thread>
#include <chrono>
#include <memory>



void simulateKeyboard(network::tcp::Socket* socket)
{
  std::this_thread::sleep_for(std::chrono::seconds(15));
  std::cout << "Shutting down socket" << std::endl;
  socket->shutdownSocket();
}


void handle_message_queue(SocketMessageQueue* message_queue)
{
  std::cout << "Message responder started" << std::endl;
  while (true)
  {
    Message message = message_queue->retrieveReceivedMessage();
    std::cout << "Received message" << std::endl;
    message_queue->enqueueResponseMessage({"200 OK", message.getSocket()});

    std::cout << "enqueued answer" << std::endl;
    break;
  }
}


int main()
{
  logging::Logger::getInstance().setLogLevel(logging::LogLevel::DEBUG);
  logging::Logger::getInstance().setLogThreadId(true);

  const logging::Trace trace(__func__ );

  SocketMessageQueue socketMessageQueue;
  network::tcp::Socket socket(network::ip::IPv4Address(127, 0, 0, 1), 8080, socketMessageQueue);

  std::thread thread(simulateKeyboard, &socket);

  socket.listenSocket();

  std::thread answer_thread(handle_message_queue, &socketMessageQueue);

  thread.join();
  answer_thread.join();

  return 0;
}
