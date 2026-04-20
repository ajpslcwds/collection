#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "dsflog.h"

// 模拟业务模块的日志输出
void worker(int id, std::atomic_bool &stop)
{
    int iter = 0;
    while (!stop)
    {
        LOG_DEBUG("worker[%d] iter=%d status=running", id, iter);
        LOG_INFO("worker[%d] processed item=%d", id, iter * 10 + id);
        if (iter % (id + 5) == 0)
        {
            LOG_WARN("worker[%d] queue depth approaching limit: %d", id, iter * 10 + id);
        }
        if (iter % (id + 13) == 0)
        {
            LOG_ERROR("worker[%d] simulated error at iter=%d", id, iter * 10 + id);
        }
        ++iter;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    LOG_INFO("worker[%d] stopped after %d iterations", id, iter);
}

int main(int argc, char *argv[])
{
    // 支持通过命令行参数指定配置文件，默认相对路径
    const char *conf = (argc > 1) ? argv[1] : "../../conf/dsflog_config.xml";

    if (LOG_INIT("process1", conf) != 0)
    {
        std::cerr << "Failed to init logger, conf: " << conf << std::endl;
        return 1;
    }

    LOG_INFO("=== demo started ===");
    LOG_INFO("config: %s", conf);
    LOG_INFO(
        "hot-reload enabled: modify CurrentLogLV or FileName in conf to take effect within ChkCfgMdfyPeriod seconds");

    // 启动 10 个工作线程
    std::atomic_bool stop{false};
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back(worker, i, std::ref(stop));
    }

    // 主线程每隔一段时间打印一次心跳，运行约 100s 后退出
    // 期间可以修改 conf/dsflog.conf 中的 CurrentLogLV 观察热重载效果
    for (int i = 0; i < 100; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (i % 2 == 0)
        {
            LOG_INFO("--- heartbeat t=%ds, tip: change CurrentLogLV in conf now ---", i + 1);
        }
    }

    stop = true;
    for (auto &t : threads)
        t.join();

    LOG_INFO("=== demo finished ===");
    LOG_SHUTDOWN();
    return 0;
}
