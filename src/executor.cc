#include "executor.h"

#include <iostream>

#include <boost/thread/latch.hpp>

namespace praster {

struct latch_wrapper {
  std::unique_ptr<boost::latch> latch_;
  bool blocked_;

  latch_wrapper(bool blocked, size_t num_threads) : blocked_(blocked) {
    if (blocked) {
      latch_ = std::make_unique<boost::latch>(num_threads);
    }
  }

  void count_down() {
    if (blocked_) {
      latch_->count_down();
    }
  }

  void wait() {
    if (blocked_) {
      latch_->wait();
    }
  }
};

executor::executor(int num_threads) : m_num_threads(num_threads) {
  m_thread_pool = std::make_unique<boost::asio::thread_pool>(m_num_threads);
}

executor::~executor() { m_thread_pool->join(); }

int executor::get_num_threads() const { return m_num_threads; }

void executor::submit(std::function<void(context)> task) {
  context c = {.task_num = 0};

  boost::asio::post(*m_thread_pool, [c, task]() { task(c); });
}

void executor::broadcast(std::function<void(context)> task, bool blocked) {
  auto latch = std::make_shared<latch_wrapper>(blocked, get_num_threads());

  for (int i = 0; i < get_num_threads(); ++i) {
    context c = {.task_num = i};
    boost::asio::post(*m_thread_pool, [c, latch, task]() {
      task(c);
      latch->count_down();
    });
  }

  latch->wait();
}

} // namespace praster
