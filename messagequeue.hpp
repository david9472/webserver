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

///@interface MessageQueue
class MessageQueue
{
public:
  ///@brief Shall add a received message to the message queue. Exclusive access to the queue must be ensured
  virtual void enqueueReceivedMessage(const Message& message) = 0;
  ///@brief Removes a previously received message from the queue. In case no message is available, the accessing thread blocks until a message is available
  virtual Message retrieveReceivedMessage() = 0;
  ///@brief Removes a previously received message from the queue. In case no message is available, an empty object gets returned immediately
  virtual std::optional<Message> retrieveResponseMessageNonBlocking() = 0;
  ///@brief Removes a previously enqueued response message from the queue. In case no message is available, the accessing thread blocks until a message is available
  virtual Message retrieveResponseMessage() = 0;
  ///@brief Shall add a response message to the message queue. Exclusive access to the queue must be ensured
  virtual void enqueueResponseMessage(const Message& message) = 0;
  ///@brief performs the shutdown procedure. All blocking synchronisation primitives must be signaled
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
