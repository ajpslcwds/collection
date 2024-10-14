
/*  Filename:    thread_pool.h
 *  Copyright:   Shanghai Baosight Software Co., Ltd.
 *
 *  Description: thread pool for async tasks
 *
 *  @author:     wuzheqiang
 *  @version:    09/10/2024	wuzheqiang	Initial Version
 **************************************************************/

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
    ThreadPool(size_t nThreads);
    ~ThreadPool();

    template <class F, class... Args>
    auto Enqueue(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>;

  private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> m_vecWorkers;
    // the task queue
    std::queue<std::function<void()>> m_queTasks;
    // synchronization
    std::mutex m_QueueMutex;
    std::condition_variable m_QueueCv;
    std::atomic_bool m_bStop = {false};
};

/**
 * @brief		the constructor just launches some amount of workers
 * @param [in]	nThreads  threads number
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
inline ThreadPool::ThreadPool(size_t nThreads)
{
    m_bStop.store(false);
    for (size_t i = 0; i < nThreads; ++i)
    {
        std::thread td([this] {
            while (!m_bStop)
            {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->m_QueueMutex);
                    this->m_QueueCv.wait(lock, [this] { return this->m_bStop || !this->m_queTasks.empty(); });
                    if (this->m_bStop && this->m_queTasks.empty())
                        return;
                    task = std::move(this->m_queTasks.front());
                    this->m_queTasks.pop();
                }
                task();
            }
        });
        std::string name = "pool_" + std::to_string(i);
        pthread_setname_np(td.native_handle(), name.c_str());
        m_vecWorkers.emplace_back(std::move(td));
    }
}

/**
 * @brief		add new work item to the pool
 * @param [in]	f  function to be executed
 * @param [in]	args  function arguments
 * @return		futurn if success
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
template <class F, class... Args>
auto ThreadPool::Enqueue(F &&f, Args &&...args) -> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task =
        std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);

        // don't allow enqueueing after stopping the pool
        if (m_bStop)
            return std::future<return_type>();

        m_queTasks.emplace([task]() { (*task)(); });
    }
    m_QueueCv.notify_one();
    return res;
}

/**
 * @brief		the destructor joins all threads
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
inline ThreadPool::~ThreadPool()
{
    if (m_bStop.exchange(true))
    {
        return;
    }
    m_QueueCv.notify_all();
    for (std::thread &worker : m_vecWorkers)
    {
        worker.join();
    }
}
} // namespace drsdk
#endif