#pragma once
#include <stdint.h>

#include <atomic>
#include <cstdarg>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace dsflog {

enum class LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL };

struct LogConf {
  std::string name;
  LogLevel log_level = LogLevel::INFO;
  bool async = true;
  int32_t file_max_kb = 0;
  int32_t file_max_no = 0;
  int32_t save_days = 0;
};

class LoggerImpl;
class Logger {
 public:
  static Logger& GetInstance() {
    static Logger instance;
    return instance;
  }
  int32_t Init(const std::string& file);
  int32_t InitLogConf();
  int32_t Shutdown();
  void LogWithMeta(LogLevel lv, const char* file, const char* func, int line, const char* fmt, ...);

 private:
  Logger() = default;
  ~Logger() = default;
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  void VLogWithMeta(LogLevel lv, const char* file, const char* func, int line, const char* fmt, va_list args);
  static LogLevel ParseLevel(const char* level_str);

 private:
  LoggerImpl* logger_impl_ = nullptr;
  std::atomic_bool inited_{false};
  std::string log_conf_file_;
  std::unordered_map<std::string, LogConf> log_confs_;
};

}  // namespace dsflog

#define LOG_INIT(file) dsflog::Logger::GetInstance().Init(file)
#define LOG_SHUTDOWN() dsflog::Logger::GetInstance().Shutdown()

#define LOG_TRACE(fmt, ...) \
  dsflog::Logger::GetInstance().LogWithMeta(dsflog::LogLevel::TRACE, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) \
  dsflog::Logger::GetInstance().LogWithMeta(dsflog::LogLevel::DEBUG, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) \
  dsflog::Logger::GetInstance().LogWithMeta(dsflog::LogLevel::INFO, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) \
  dsflog::Logger::GetInstance().LogWithMeta(dsflog::LogLevel::WARN, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) \
  dsflog::Logger::GetInstance().LogWithMeta(dsflog::LogLevel::ERROR, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_CRIT(fmt, ...) \
  dsflog::Logger::GetInstance().LogWithMeta(dsflog::LogLevel::CRITICAL, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
