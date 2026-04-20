#pragma once
#include <stdint.h>

#include <atomic>
#include <cstdarg>
#include <ctime>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

namespace dsflog
{

enum class LogLevel
{
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};

struct LogConf
{
    std::string name;
    std::string filename;
    std::string pattern;
    LogLevel log_level = LogLevel::INFO;
    bool async = true;
    bool log_on_monitor = false;
    int32_t file_max_kb = 0;
    int32_t file_max_no = 0;
    int32_t save_days = 0;
};

class LoggerImpl;
class Logger
{
  public:
    static Logger &GetInstance()
    {
        static Logger instance;
        return instance;
    }
    int32_t Init(const std::string &process_name, const std::string &conf_file);
    int32_t InitLogConf();
    int32_t Shutdown();
    void LogWithMeta(LogLevel lv, const char *file, const char *func, int line, const char *fmt, ...);

  private:
    Logger() = default;
    ~Logger()
    {
        Shutdown();
    }
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    void VLogWithMeta(LogLevel lv, const char *file, const char *func, int line, const char *fmt, va_list args);
    static LogLevel ParseLevel(const char *level_str);
    void ReloadLoop();
    void CheckAndReload();

  private:
    LoggerImpl *logger_impl_ = nullptr;
    std::atomic_bool inited_{false};
    std::atomic_bool running_{false};
    std::string process_name_;
    std::string log_conf_file_;
    std::unordered_map<std::string, LogConf> log_confs_;
    int32_t check_period_s_{10};
    time_t last_mtime_{0};
    std::thread reload_thread_;
};

} // namespace dsflog

#define LOG_INIT(name, file) dsflog::Logger::GetInstance().Init(name, file)
#define LOG_SHUTDOWN() dsflog::Logger::GetInstance().Shutdown()

#define LOG_TRACE(fmt, ...)                                                                                            \
    dsflog::Logger::GetInstance().LogWithMeta(dsflog::LogLevel::TRACE, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)                                                                                            \
    dsflog::Logger::GetInstance().LogWithMeta(dsflog::LogLevel::DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)                                                                                             \
    dsflog::Logger::GetInstance().LogWithMeta(dsflog::LogLevel::INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)                                                                                             \
    dsflog::Logger::GetInstance().LogWithMeta(dsflog::LogLevel::WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)                                                                                            \
    dsflog::Logger::GetInstance().LogWithMeta(dsflog::LogLevel::ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_CRIT(fmt, ...)                                                                                             \
    dsflog::Logger::GetInstance().LogWithMeta(dsflog::LogLevel::CRITICAL, __FILE__, __func__, __LINE__, fmt,           \
                                              ##__VA_ARGS__)
