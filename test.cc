#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>

class Timer
{
  public:
    using Callback = std::function<void()>;

    // 构造函数：接受周期（毫秒）和回调函数
    Timer(int period_ms, Callback callback)
        : period_(std::chrono::milliseconds(period_ms)), callback_(callback), running_(false)
    {
    }

    // 启动定时器
    void start()
    {
        if (running_)
            return; // 避免重复启动
        running_ = true;
        thread_ = std::thread([this] {
            auto next_time = std::chrono::steady_clock::now();
            while (running_)
            {
                // 执行回调
                callback_();

                // 计算下一次触发时间
                next_time += period_;

                // 睡眠直到下一次触发
                std::this_thread::sleep_until(next_time);
            }
        });
    }

    // 停止定时器
    void stop()
    {
        running_ = false;
        if (thread_.joinable())
        {
            thread_.join(); // 等待线程结束
        }
    }

    // 析构函数：确保线程停止
    ~Timer()
    {
        stop();
    }

  private:
    std::chrono::steady_clock::duration period_; // 周期
    Callback callback_;                          // 回调函数
    std::atomic<bool> running_;                  // 运行状态
    std::thread thread_;                         // 定时器线程
};

int main()
{
    // 示例：每 1ms 打印一次
    int count = 0;
    Timer timer(1, [&count]() {
        static int last = 0;
        int cur =
            std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now().time_since_epoch())
                .count();
        if (last != 0 && cur - last > 1200)
        {
            std::cout << "Missed " << cur << "\t" << last << std::endl;
        }

        // std::cout << " Timer triggered: " << ++count << std::endl;
        last = cur;
    });

    timer.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    timer.stop();

    std::cout << "Timer stopped" << std::endl;
    return 0;
}