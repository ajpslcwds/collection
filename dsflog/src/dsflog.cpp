#include "dsflog.h"

#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <unordered_map>
#include <cstring>

namespace dsflog {

static const char* LogLevelToStr(LogLevel lv) {
  switch (lv) {
    case LogLevel::TRACE: return "trace";
    case LogLevel::DEBUG: return "debug";
    case LogLevel::INFO: return "info";
    case LogLevel::WARN: return "warn";
    case LogLevel::ERROR: return "error";
    case LogLevel::CRITICAL: return "critical";
    default: return "info";
  }
}

static spdlog::level::level_enum ToSpdlogLevel(LogLevel lv) {
  switch (lv) {
    case LogLevel::TRACE: return spdlog::level::trace;
    case LogLevel::DEBUG: return spdlog::level::debug;
    case LogLevel::INFO: return spdlog::level::info;
    case LogLevel::WARN: return spdlog::level::warn;
    case LogLevel::ERROR: return spdlog::level::err;
    case LogLevel::CRITICAL: return spdlog::level::critical;
    default: return spdlog::level::info;
  }
}

LogLevel Logger::ParseLevel(const char* level_str) {
  if (level_str == nullptr) return LogLevel::INFO;
  if (strcmp(level_str, "TRACE") == 0) return LogLevel::TRACE;
  if (strcmp(level_str, "DEBUG") == 0) return LogLevel::DEBUG;
  if (strcmp(level_str, "INFO") == 0) return LogLevel::INFO;
  if (strcmp(level_str, "WARN") == 0 || strcmp(level_str, "WARNING") == 0) return LogLevel::WARN;
  if (strcmp(level_str, "ERROR") == 0) return LogLevel::ERROR;
  if (strcmp(level_str, "CRITICAL") == 0) return LogLevel::CRITICAL;
  return LogLevel::INFO;
}

class LoggerImpl {
 public:
  LoggerImpl() = default;
  ~LoggerImpl() { Shutdown(); }

  int32_t Init(const std::unordered_map<std::string, LogConf>& confs) {
    auto it = confs.find("demo");
    if (it == confs.end()) {
      it = confs.begin();
    }

    if (it != confs.end()) {
      default_logger_ = CreateLogger(it->second);
      if (default_logger_) {
        spdlog::set_default_logger(default_logger_);
      }
    }

    if (!default_logger_) {
      auto console_logger = spdlog::stdout_color_mt("console");
      console_logger->set_level(spdlog::level::info);
      console_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");
      default_logger_ = console_logger;
      spdlog::set_default_logger(default_logger_);
    }

    return 0;
  }

  void Shutdown() {
    default_logger_.reset();
    spdlog::shutdown();
  }

  void Log(LogLevel lv, const char* file, const char* func, int line, const char* fmt, va_list args) {
    if (!default_logger_) {
      return;
    }

    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    std::string msg = fmt::format("[{}:{}:{}]{}", file, line,func, buffer);
    auto spd_level = ToSpdlogLevel(lv);
    default_logger_->log(spd_level, msg);
  }

 private:
  std::shared_ptr<spdlog::logger> CreateLogger(const LogConf& conf) {
    try {
      auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

      spdlog::sink_ptr file_sink = nullptr;
      std::string log_file = conf.name + ".log";

      if (conf.save_days > 0) {
        file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(log_file, 0, 0);
      } else if (conf.file_max_kb > 0) {
        file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            log_file, conf.file_max_kb * 1024, conf.file_max_no);
      }

      std::shared_ptr<spdlog::logger> logger;
      if (file_sink) {
        spdlog::sinks_init_list sink_list = {console_sink, file_sink};
        logger = std::make_shared<spdlog::logger>(conf.name, sink_list);
      } else {
        logger = std::make_shared<spdlog::logger>(conf.name, console_sink);
      }

      logger->set_level(ToSpdlogLevel(conf.log_level));
      logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v");

      spdlog::info("Created logger: name={}, level={}", conf.name, LogLevelToStr(conf.log_level));
      return logger;

    } catch (const spdlog::spdlog_ex& ex) {
      spdlog::error("Logger creation failed: {}", ex.what());
      return nullptr;
    }
  }

 private:
  std::shared_ptr<spdlog::logger> default_logger_;
};

int32_t Logger::Init(const std::string& file) {
  if (inited_.exchange(true)) {
    return 0;
  }
  log_conf_file_ = file;
  int32_t ret = InitLogConf();
  if (ret != 0) {
    return ret;
  }

  logger_impl_ = new LoggerImpl();
  return logger_impl_->Init(log_confs_);
}

int32_t Logger::InitLogConf() {
  using namespace tinyxml2;

  XMLDocument doc;
  XMLError err = doc.LoadFile(log_conf_file_.c_str());
  if (err != XML_SUCCESS) {
    spdlog::error("Failed to load config file: {}", log_conf_file_);
    return -1;
  }

  auto* root = doc.FirstChildElement("LogConf");
  if (root == nullptr) {
    spdlog::error("Failed to find <LogConf> element");
    return -1;
  }

  for (auto* e = root->FirstChildElement(); e; e = e->NextSiblingElement()) {
    LogConf conf;
    conf.name = e->Name();

    const char* lv = e->Attribute("CurrentLogLV");
    conf.log_level = ParseLevel(lv);
    conf.async = e->IntAttribute("Async", 1) > 0;
    conf.file_max_kb = e->IntAttribute("FileMaxKb", 1024 * 100);
    conf.file_max_no = e->IntAttribute("FileMaxNo", 3);
    conf.save_days = e->IntAttribute("SaveDays", 0);

    log_confs_[conf.name] = conf;
  }

  return 0;
}

int32_t Logger::Shutdown() {
  if (logger_impl_ != nullptr) {
    logger_impl_->Shutdown();
    delete logger_impl_;
    logger_impl_ = nullptr;
  }
  log_confs_.clear();
  inited_ = false;
  return 0;
}

void Logger::LogWithMeta(LogLevel lv, const char* file, const char* func, int line, const char* fmt, ...) {
  if (!inited_) {
    return;
  }
  va_list args;
  va_start(args, fmt);
  VLogWithMeta(lv, file, func, line, fmt, args);
  va_end(args);
}

void Logger::VLogWithMeta(LogLevel lv, const char* file, const char* func, int line, const char* fmt, va_list args) {
  if (!inited_ || logger_impl_ == nullptr) {
    return;
  }
  logger_impl_->Log(lv, file, func, line, fmt, args);
}

}  // namespace dsflog
