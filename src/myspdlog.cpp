#include "myspdlog.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <vector>

namespace myspdlog
{

static std::shared_ptr<spdlog::logger> g_logger;
static std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> g_file_sink;

void init(LogLevel level, bool console)
{
    try
    {
        // 初始化异步日志线程池
        spdlog::init_thread_pool(8192, 1); // 队列大小 8192，1 个工作线程

        std::vector<spdlog::sink_ptr> sinks;
        // 如果启用控制台输出，添加控制台 sink
        if (console)
        {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(spdlog::level::debug);
            sinks.push_back(console_sink);
        }

        // 创建 logger
        g_logger = std::make_shared<spdlog::logger>("myspdlog", sinks.begin(), sinks.end());

        // 设置日志级别
        switch (level)
        {
        case LogLevel::TRACE:
            g_logger->set_level(spdlog::level::trace);
            break;
        case LogLevel::DEBUG:
            g_logger->set_level(spdlog::level::debug);
            break;
        case LogLevel::INFO:
            g_logger->set_level(spdlog::level::info);
            break;
        case LogLevel::WARN:
            g_logger->set_level(spdlog::level::warn);
            break;
        case LogLevel::ERROR:
            g_logger->set_level(spdlog::level::err);
            break;
        case LogLevel::CRITICAL:
            g_logger->set_level(spdlog::level::critical);
            break;
        }

        // 设置日志格式，包含文件、函数和行号
        g_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%t][%l]%v");
        g_logger->flush_on(spdlog::level::debug); // 全局刷新
        spdlog::register_logger(g_logger);
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cerr << "Log initialization failed: " << ex.what() << std::endl;
    }
}

void set_log_file(const std::string &log_file, const LogLevel level, const int max_size /*MB*/, const int max_files)
{
    try
    {
        if (!g_logger)
        {
            std::cerr << "Logger not initialized" << std::endl;
            return;
        }

        // 获取当前 sinks
        auto sinks = g_logger->sinks();

        // 移除现有文件 sink
        sinks.erase(std::remove_if(sinks.begin(), sinks.end(),
                                   [](const spdlog::sink_ptr &sink) {
                                       return dynamic_cast<spdlog::sinks::rotating_file_sink_mt *>(sink.get()) !=
                                              nullptr;
                                   }),
                    sinks.end());

        // 如果 log_file 不为空，添加新的文件 sink
        if (!log_file.empty())
        {
            g_file_sink =
                std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file, max_size * 1024 * 1024, max_files);
            g_file_sink->set_level(spdlog::level::trace);
            sinks.push_back(g_file_sink);
        }
        else
        {
            g_file_sink.reset();
        }

        auto level = g_logger->level();
        g_logger = std::make_shared<spdlog::async_logger>("myspdlog", sinks.begin(), sinks.end(), spdlog::thread_pool(),
                                                          spdlog::async_overflow_policy::block);
        g_logger->set_level(level);
        g_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e][%t][%l]%v");
        g_logger->flush_on(spdlog::level::trace);
        spdlog::register_logger(g_logger);
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        std::cerr << "Set log file failed: " << ex.what() << std::endl;
    }
}

void shutdown()
{
    if (g_logger)
    {
        g_logger->flush(); // 确保关闭前刷新
        g_logger.reset();
        g_file_sink.reset();
        spdlog::drop_all();
    }
}

void trace(const std::string &message)
{
    if (g_logger)
        g_logger->trace(message);
}

void debug(const std::string &message)
{
    if (g_logger)
        g_logger->debug(message);
}

void info(const std::string &message)
{
    if (g_logger)
        g_logger->info(message);
}

void warn(const std::string &message)
{
    if (g_logger)
        g_logger->warn(message);
}

void error(const std::string &message)
{
    if (g_logger)
        g_logger->error(message);
}

void critical(const std::string &message)
{
    if (g_logger)
        g_logger->critical(message);
}

// LogStream 实现
LogStream::LogStream(LogLevel level, const char *file, const char *func, int line)
    : level_(level), file_(file), func_(func), line_(line)
{
}

LogStream::~LogStream()
{
    std::string filename = std::string(file_).substr(std::string(file_).find_last_of("/\\") + 1);
    if (g_logger)
    {
        std::string message = stream_.str();
        switch (level_)
        {
        case LogLevel::TRACE:
            g_logger->trace("[{}:{}:{}] {}", filename, line_, func_, message);
            break;
        case LogLevel::DEBUG:
            g_logger->debug("[{}:{}:{}] {}", filename, line_, func_, message);
            break;
        case LogLevel::INFO:
            g_logger->info("[{}:{}:{}] {}", filename, line_, func_, message);
            break;
        case LogLevel::WARN:
            g_logger->warn("[{}:{}:{}] {}", filename, line_, func_, message);
            break;
        case LogLevel::ERROR:
            g_logger->error("[{}:{}:{}] {}", filename, line_, func_, message);
            break;
        case LogLevel::CRITICAL:
            g_logger->critical("[{}:{}:{}] {}", filename, line_, func_, message);
            break;
        }
    }
}

} // namespace myspdlog