#ifndef PRASTER_EXECUTOR_H
#define PRASTER_EXECUTOR_H

#include <functional>
#include <memory>

#include <boost/asio.hpp>

namespace praster {

class executor {
public:
  struct context {
    int task_num;
  };

  explicit executor(int num_threads);
  ~executor();

  int get_num_threads() const;
  void submit(std::function<void(context)> task);
  void broadcast(std::function<void(context)> task, bool blocked);

private:
  int m_num_threads;
  std::unique_ptr<boost::asio::thread_pool> m_thread_pool;
};
} // namespace praster

#endif