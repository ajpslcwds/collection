
//
// g++ test.cpp -I /home/wzq/code/collection/build/install/include -L /home/wzq/code/collection/build/install/lib
// -lmyspdlog -lfmt  -o test
//
#include <myspdlog.h>
#include <thread>
#include <vector>
int main()
{

    myspdlog::init(myspdlog::LogLevel::INFO, true);
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++)
    {
        threads.emplace_back([i]() {
            for (int k = 0; k < 10000; k++)
            {
                myspdlog::debug("debug log");
                myspdlog::info("info log");
                myspdlog::warn("warn log");
                myspdlog::error("error log");
                SPDLOG_DEBUG << "debug log" << "debug log" << std::endl;
                SPDLOG_INFO << "info log" << "info log" << std::endl;
                SPDLOG_WARN << "warn log" << "warn log" << std::flush;
                SPDLOG_ERROR << k << " error log" << "error log";
                if (i == 0)
                {
                    if (k == 2000)
                    {
                        myspdlog::set_log_file("test22.log");
                    }
                    else if (k == 4000)
                    {
                        myspdlog::set_log_file("test33.log");
                    }
                    else if (k == 6000)
                    {
                        myspdlog::set_log_file("");
                    }
                    else if (k == 8000)
                    {
                        myspdlog::set_log_file("test44.log");
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }

    for (auto &t : threads)
    {
        t.join();
    }

    myspdlog::shutdown();
    return 0;
}
