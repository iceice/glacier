#ifndef GLACIER_BASE_THREADPOOL_
#define GLACIER_BASE_THREADPOOL_

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <vector>

namespace glacier {

//
// 使用c++11实现一个线程池
//
class ThreadPool {
 public:
  typedef std::thread Thread;
  typedef std::function<void()> Task;
  typedef std::unique_lock<std::mutex> UniqueLock;

  //
  // 构造函数
  //
  ThreadPool(int numThread) : stoped_(false) {
    for (int i = 0; i < numThread; ++i) {
      threads_.emplace_back([this] {
        while (!this->stoped_) {
          Task task;
          {
            UniqueLock lock(this->mtx_);
            // 等待条件触发，当线程池停止或者任务队列不空的时候，停止等待
            this->condition_.wait(lock, [this] {
              return this->stoped_ || !this->tasks_.empty();
            });
            // 如果线程池停止了，并且任务队列为空，函数返回
            if (this->stoped_ && this->tasks_.empty()) {
              return;
            }
            task = std::move(this->tasks_.front());
            this->tasks_.pop();
          }
          task();
        }
      });
    }
  }

  //
  // 析构函数
  //
  ~ThreadPool() {
    stoped_.store(true);
    condition_.notify_all();
    for (Thread& t : threads_) {
      t.join();
    }
  }

  //
  // 提交一个模板任务
  //
  template <class F, class... Args>
  auto commit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    if (stoped_.load()) throw std::runtime_error("commit on stoped threadpool");

    using return_type = decltype(f(args...));
    auto task = std::make_shared<std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> res = task->get_future();
    // 添加任务到队列
    {
      UniqueLock lock(mtx_);
      tasks_.emplace([task] { (*task)(); });
    }
    condition_.notify_one();
    return res;
  }

  //
  // 提交一个void任务
  //
  void commit(Task task) {
    if (threads_.empty()) {
      task();
    } else {
      UniqueLock lock(mtx_);
      tasks_.emplace(std::move(task));
      condition_.notify_one();
    }
  }

 private:
  std::vector<Thread> threads_;        // 线程数组
  std::queue<Task> tasks_;             // 任务队列
  std::mutex mtx_;                     // 互斥量
  std::condition_variable condition_;  // 条件变量
  std::atomic<bool> stoped_;           // 停止标志
};

}  // namespace glacier

#endif  // GLACIER_BASE_THREADPOOL_