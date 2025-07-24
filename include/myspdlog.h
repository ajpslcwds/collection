#ifndef MYSPDLOG_H
#define MYSPDLOG_H

#include <sstream>
#include <string>

namespace myspdlog
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

// 初始化日志系统
void init(LogLevel level = LogLevel::INFO, bool console = true);

// 动态设置或禁用文件日志（空字符串表示禁用）
void set_log_file(const std::string &log_file, const LogLevel level = LogLevel::INFO, const int max_size = 1 /*MB*/,
                  const int max_files = 5);

// 清理日志系统
void shutdown();

// 日志输出函数（保留原有接口）
void trace(const std::string &message);
void debug(const std::string &message);
void info(const std::string &message);
void warn(const std::string &message);
void error(const std::string &message);
void critical(const std::string &message);

// 流式日志类
class LogStream
{
  public:
    LogStream(LogLevel level, const char *file, const char *func, int line);
    ~LogStream();

    // 模板函数支持 << 操作符
    template <typename T> LogStream &operator<<(const T &value)
    {
        stream_ << value;
        return *this;
    }
    // for std::endl,std::flush
    LogStream &operator<<(std::ostream &(*pf)(std::ostream &))
    {
        {
            // Not processing; When the final output , std::endl will be added;
            // pf(stream_);
        }
        return *this;
    }

  private:
    LogLevel level_;
    std::ostringstream stream_;
    const char *file_;
    const char *func_;
    int line_;
};

// 宏定义，类似 dsfapi::LogStream
#define SPDLOG_TRACE myspdlog::LogStream(myspdlog::LogLevel::TRACE, __FILE__, __func__, __LINE__)
#define SPDLOG_DEBUG myspdlog::LogStream(myspdlog::LogLevel::DEBUG, __FILE__, __func__, __LINE__)
#define SPDLOG_INFO myspdlog::LogStream(myspdlog::LogLevel::INFO, __FILE__, __func__, __LINE__)
#define SPDLOG_WARN myspdlog::LogStream(myspdlog::LogLevel::WARN, __FILE__, __func__, __LINE__)
#define SPDLOG_ERROR myspdlog::LogStream(myspdlog::LogLevel::ERROR, __FILE__, __func__, __LINE__)
#define SPDLOG_CRITICAL myspdlog::LogStream(myspdlog::LogLevel::CRITICAL, __FILE__, __func__, __LINE__)

} // namespace myspdlog

#endif // MYSPDLOG_H