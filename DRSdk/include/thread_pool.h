#ifndef DRSDK_THREAD_POOL_H
#define DRSDK_THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace drsdk
{

class ThreadPool
{
  public:
    ThreadPool(size_t threads);
    ~ThreadPool();

    template <class F, class... Args>
    auto enqueue(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>;

  private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> workers_;
    // the task queue
    std::queue<std::function<void()>> tasks_;
    // synchronization
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic_bool stop_ = {false};
};

// the constructor just launches some amount of workers
inline ThreadPool::ThreadPool(size_t threads)
{
    stop_.store(false);
    for (size_t i = 0; i < threads; ++i)
        workers_.emplace_back([this] {
            while (!stop_)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex_);
                    this->queue_cv_.wait(lock, [this] { return this->stop_ || !this->tasks_.empty(); });
                    if (this->stop_ && this->tasks_.empty())
                        return;
                    task = std::move(this->tasks_.front());
                    this->tasks_.pop();
                }
                task();
            }
        });
}

// add new work item to the pool
template <class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task =
        std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);

        // don't allow enqueueing after stopping the pool
        if (stop_)
            return std::future<return_type>();

        tasks_.emplace([task]() { (*task)(); });
    }
    queue_cv_.notify_one();
    return res;
}

// the destructor joins all threads
inline ThreadPool::~ThreadPool()
{
    if (stop_.exchange(true))
    {
        return;
    }
    queue_cv_.notify_all();
    for (std::thread &worker : workers_)
    {
        worker.join();
    }
}
} // namespace drsdk
#endif