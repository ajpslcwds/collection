#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
  public:
    // 构造函数，创建线程池
    ThreadPool(size_t thread_count) : stop_flag(false)
    {
        for (size_t i = 0; i < thread_count; ++i)
        {
            workers.emplace_back([this] {
                while (true)
                {
                    std::function<void()> task;

                    { // 临界区
                        std::unique_lock<std::mutex> lock(this->queue_mutex);

                        // 等待任务或停止信号
                        this->condition.wait(lock, [this] { return this->stop_flag || !this->tasks.empty(); });

                        // 如果是停止信号并且任务队列为空，退出循环
                        if (this->stop_flag && this->tasks.empty())
                        {
                            return;
                        }

                        // 获取一个任务
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }

                    // 执行任务
                    task();
                }
            });
        }
    }

    // 向任务队列中添加任务
    template <typename F, typename... Args> void Enqueue(F &&f, Args &&...args)
    {
        { // 临界区
            std::unique_lock<std::mutex> lock(queue_mutex);

            // 将任务打包成 std::function<void()>
            tasks.emplace([f, args...]() { f(args...); });
        }
        condition.notify_one(); // 通知一个线程来处理任务
    }

    // 析构函数，清理线程池
    ~ThreadPool()
    {
        { // 临界区
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop_flag = true;
        }

        condition.notify_all(); // 通知所有线程停止
        for (std::thread &worker : workers)
        {
            worker.join(); // 等待所有线程完成
        }
    }

  private:
    std::vector<std::thread> workers;        // 工作线程
    std::queue<std::function<void()>> tasks; // 任务队列

    std::mutex queue_mutex;            // 互斥锁，保护任务队列
    std::condition_variable condition; // 条件变量，通知工作线程
    std::atomic<bool> stop_flag;       // 标志位，指示线程池是否停止
};

int main()
{
    // 创建一个包含4个线程的线程池
    ThreadPool pool(4);

    // 提交一些任务
    pool.Enqueue([] { std::cout << "Task 1 executed\n"; });
    pool.Enqueue([] { std::cout << "Task 2 executed\n"; });
    pool.Enqueue([] { std::cout << "Task 3 executed\n"; });
    pool.Enqueue([](int x) { std::cout << "Task 4 executed with argument: " << x << "\n"; }, 42);
    pool.Enqueue([] { std::cout << "Task 1 executed\n"; });
    pool.Enqueue([] { std::cout << "Task 2 executed\n"; });
    pool.Enqueue([] { std::cout << "Task 3 executed\n"; });
    pool.Enqueue([](int x) { std::cout << "Task 4 executed with argument: " << x << "\n"; }, 42);

    // 稍微等待一些时间，确保任务执行完毕
    std::this_thread::sleep_for(std::chrono::seconds(1));

    return 0;
}
