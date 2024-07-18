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

namespace container::message_queue
{
  class Message
  {
  private:
    std::string msg_;
    int socket_;
  public:
    Message(std::string msg, int socket) : msg_(std::move(msg)), socket_(socket)
    {}

    [[nodiscard]] const std::string &getMessageString() const
    { return msg_; }

    [[nodiscard]] int getSocket() const
    { return socket_; }
  };

///@interface MessageQueue
  class Queue
  {
  public:
    ///@brief Shall add a received message to the message queue. Exclusive access to the queue must be ensured
    virtual void enqueueReceivedMessage(const Message &message) = 0;

    ///@brief Removes a previously received message from the queue. In case no message is available, the accessing thread blocks until a message is available
    virtual Message retrieveReceivedMessage() = 0;

    ///@brief Removes a previously received message from the queue. In case no message is available, an empty object gets returned immediately
    virtual std::optional<Message> retrieveResponseMessageNonBlocking() = 0;

    ///@brief Removes a previously enqueued response message from the queue. In case no message is available, the accessing thread blocks until a message is available
    virtual Message retrieveResponseMessage() = 0;

    ///@brief Shall add a response message to the message queue. Exclusive access to the queue must be ensured
    virtual void enqueueResponseMessage(const Message &message) = 0;

    ///@brief performs the shutdown procedure. All blocking synchronisation primitives must be signaled
    virtual void shutdown() = 0;
  };
}
namespace network::tcp
{
  class SocketMessageQueue : public container::message_queue::Queue
  {
    std::mutex received_queue_mutex_;
    std::condition_variable received_queue_cv_;
    std::queue<container::message_queue::Message> received_queue_;

    std::mutex respond_queue_mutex_;
    std::condition_variable respond_queue_cv_;
    std::queue<container::message_queue::Message> respond_queue_;

    std::mutex shutdown_mutex_;
    bool shutdown_{false};
  public:
    void enqueueReceivedMessage(const container::message_queue::Message &message) override;

    container::message_queue::Message retrieveReceivedMessage() override;

    std::optional<container::message_queue::Message> retrieveResponseMessageNonBlocking() override;

    container::message_queue::Message retrieveResponseMessage() override;

    void enqueueResponseMessage(const container::message_queue::Message &message) override;

    void shutdown() override;
  };
}








#endif //WEBSERVER_MESSAGEQUEUE_HPP
