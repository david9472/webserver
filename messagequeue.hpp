//
// Created by david on 13/07/24.
//

#ifndef WEBSERVER_MESSAGEQUEUE_HPP
#define WEBSERVER_MESSAGEQUEUE_HPP

#include <string>
#include <queue>
#include <optional>
#include <utility>
#include <mutex>
#include <condition_variable>

class Message
{
private:
  std::string msg_;
  int socket_;
public:
  Message(std::string msg, int socket) : msg_(std::move(msg)), socket_(socket) {}

  [[nodiscard]] const std::string& getMessageString() const { return msg_; }
  [[nodiscard]] int getSocket() const { return socket_; }
};

class MessageQueue
{
public:
  virtual void enqueueReceivedMessage(const Message& message) = 0;
  virtual Message retrieveReceivedMessage() = 0;

  virtual std::optional<Message> retrieveResponseMessageNonBlocking() = 0;
  virtual Message retrieveResponseMessage() = 0;

  virtual void enqueueResponseMessage(const Message& message) = 0;

  virtual void shutdown() = 0;
};

class SocketMessageQueue : public MessageQueue
{
  std::mutex received_queue_mutex_;
  std::condition_variable received_queue_cv_;
  std::queue<Message> received_queue_;

  std::mutex respond_queue_mutex_;
  std::condition_variable respond_queue_cv_;
  std::queue<Message> respond_queue_;

  std::mutex shutdown_mutex_;
  bool shutdown_{false};
public:
  void enqueueReceivedMessage(const Message& message) override;
  Message retrieveReceivedMessage() override;

  std::optional<Message> retrieveResponseMessageNonBlocking() override;
  Message retrieveResponseMessage() override;

  void enqueueResponseMessage(const Message& message) override;

  void shutdown() override;
};








#endif //WEBSERVER_MESSAGEQUEUE_HPP
