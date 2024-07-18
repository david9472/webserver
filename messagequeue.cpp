//
// Created by david on 13/07/24.
//

#include "messagequeue.hpp"

#include "error.hpp"
#include "logger.hpp"
#include "trace.hpp"

namespace network::tcp
{
  void SocketMessageQueue::enqueueReceivedMessage(const container::message_queue::Message &message)
  {
    const logging::Trace trace(__func__);
    received_queue_mutex_.lock();
    received_queue_.emplace(message);
    received_queue_mutex_.unlock();

    received_queue_cv_.notify_one();
  }

  container::message_queue::Message SocketMessageQueue::retrieveReceivedMessage()
  {
    const logging::Trace trace(__func__);
    std::unique_lock<std::mutex> unique_received_queue_lock(received_queue_mutex_);

    while (received_queue_.empty())
    {
      received_queue_cv_.wait(unique_received_queue_lock);

      std::lock_guard guard_shutdown_lock(shutdown_mutex_);
      if (shutdown_)
      {
        unique_received_queue_lock.unlock();
        return {"", -1};
      }
    }

    if (!unique_received_queue_lock.owns_lock())
      throw logging::Error(LOC, "receiving mutex not locked properly!");

    if (received_queue_.empty())
      throw logging::Error(LOC, "retrieving from an empty queue is not allowed!");

    const container::message_queue::Message message{received_queue_.front()};
    received_queue_.pop();

    unique_received_queue_lock.unlock();

    return message;
  }

  std::optional<container::message_queue::Message> SocketMessageQueue::retrieveResponseMessageNonBlocking()
  {
    const logging::Trace trace(__func__);
    std::lock_guard<std::mutex> guard_response_queue_lock{respond_queue_mutex_};
    if (respond_queue_.empty())
      return {};

    container::message_queue::Message response{respond_queue_.front()};
    respond_queue_.pop();

    return response;
  }

  container::message_queue::Message SocketMessageQueue::retrieveResponseMessage()
  {
    const logging::Trace trace(__func__);
    std::unique_lock<std::mutex> unique_respond_queue_lock(respond_queue_mutex_);

    while (respond_queue_.empty())
    {
      respond_queue_cv_.wait(unique_respond_queue_lock);

      std::lock_guard guard_shutdown_lock(shutdown_mutex_);
      if (shutdown_)
      {
        unique_respond_queue_lock.unlock();
        return {"", -1};
      }
    }

    if (!unique_respond_queue_lock.owns_lock())
      throw logging::Error(LOC, "receiving mutex not locked properly!");

    if (respond_queue_.empty())
      throw logging::Error(LOC, "retrieving from an empty queue is not allowed!");

    const container::message_queue::Message response{respond_queue_.front()};
    respond_queue_.pop();

    unique_respond_queue_lock.unlock();

    return response;
  }

  void SocketMessageQueue::enqueueResponseMessage(const container::message_queue::Message &message)
  {
    const logging::Trace trace(__func__);
    respond_queue_mutex_.lock();
    respond_queue_.emplace(message);
    respond_queue_mutex_.unlock();

    respond_queue_cv_.notify_one();
  }

  void SocketMessageQueue::shutdown()
  {
    const logging::Trace trace(__func__);
    shutdown_mutex_.lock();
    shutdown_ = true;
    shutdown_mutex_.unlock();

    respond_queue_cv_.notify_all();
    received_queue_cv_.notify_all();
  }
}