#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <stdint.h>
#include <thread>

int64_t GetCurMillsSeconds()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}
class Timer
{
  public:
    Timer() : running_(false)
    {
    }
    ~Timer()
    {
        stop();
    }

    // 启动定时器（周期执行）
    void start_1(std::function<void()> func, std::chrono::microseconds interval, bool repeat = true)
    {
        stop(); // 若已在运行则先停止
        running_ = true;
        thread_ = std::thread([=]() {
            do
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if (cv_.wait_for(lock, interval, [&]() { return !running_; }))
                    break; // 被 stop 中断
                func();
            } while (repeat && running_);
        });
    }
    void start_2(std::function<void()> func, std::chrono::microseconds interval, bool repeat = true)
    {
        stop();
        running_ = true;
        thread_ = std::thread([=]() {
            auto next = std::chrono::steady_clock::now() + interval;
            while (running_)
            {
                std::this_thread::sleep_until(next);
                next += interval;
                if (!running_)
                    break;

                func();

                if (!repeat)
                    break;
            }
        });
    }
    void start_3(std::function<void()> func, std::chrono::microseconds interval, bool repeat = true)
    {
        stop();
        running_ = true;
        thread_ = std::thread([=]() {
            auto next = std::chrono::steady_clock::now();
            while (running_)
            {
                next += interval;
                std::this_thread::sleep_until(next);

                if (!running_)
                    break;

                func();

                if (!repeat)
                    break;
            }
        });
    }

    void start_4(std::chrono::microseconds interval, bool repeat = true)
    {
        stop();
        running_ = true;
        thread_ = std::thread([=]() {
            while (running_)
            {
                auto begin = std::chrono::steady_clock::now();
                std::this_thread::sleep_until(begin + interval);
                auto end = std::chrono::steady_clock::now();
                if (end - begin >= interval * 1.6)
                {
                    std::cout << "Timer4 tick: "
                              << std::chrono::duration_cast<std::chrono::microseconds>(begin.time_since_epoch()).count()
                              << " ,"
                              << std::chrono::duration_cast<std::chrono::microseconds>(end.time_since_epoch()).count()
                              << std::endl;
                }

                if (!running_)
                    break;

                if (!repeat)
                    break;
            }
        });
    }

    // 停止定时器
    void stop()
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            running_ = false;
        }
        cv_.notify_all();
        if (thread_.joinable())
            thread_.join();
    }

    // 是否正在运行
    bool isRunning() const
    {
        return running_;
    }

  private:
    std::atomic<bool> running_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
};

int main()
{

    Timer timer;

    // timer.start_1([] { std::cout << "Timer1 tick: " << GetCurMillsSeconds() << std::endl; },
    //               std::chrono::milliseconds(10), true);
    // std::this_thread::sleep_for(std::chrono::seconds(2)); // 主线程休眠

    // timer.start_2([] { std::cout << "Timer2 tick: " << GetCurMillsSeconds() << std::endl; },
    //               std::chrono::milliseconds(2), true);
    // std::this_thread::sleep_for(std::chrono::seconds(2)); // 主线程休眠

    // timer.start_3([] { std::cout << "Timer3 tick: " << GetCurMillsSeconds() << std::endl; },
    //               std::chrono::milliseconds(1), true);
    // std::this_thread::sleep_for(std::chrono::seconds(2)); // 主线程休眠

    timer.start_4(std::chrono::microseconds(1000), true);
    std::this_thread::sleep_for(std::chrono::seconds(2)); // 主线程休眠

    timer.stop();
    std::cout << "Timer stopped." << std::endl;
    return 0;
}
