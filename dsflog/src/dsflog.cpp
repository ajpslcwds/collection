#include "dsflog.h"

#include <spdlog/async.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <tinyxml2.h>

#include <chrono>
#include <cstring>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

namespace dsflog
{

static const char *kDefaultPattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%t] %v";

static const char *LogLevelToStr(LogLevel lv)
{
    switch (lv)
    {
    case LogLevel::TRACE:
        return "trace";
    case LogLevel::DEBUG:
        return "debug";
    case LogLevel::INFO:
        return "info";
    case LogLevel::WARN:
        return "warn";
    case LogLevel::ERROR:
        return "error";
    case LogLevel::CRITICAL:
        return "critical";
    default:
        return "info";
    }
}

static spdlog::level::level_enum ToSpdlogLevel(LogLevel lv)
{
    switch (lv)
    {
    case LogLevel::TRACE:
        return spdlog::level::trace;
    case LogLevel::DEBUG:
        return spdlog::level::debug;
    case LogLevel::INFO:
        return spdlog::level::info;
    case LogLevel::WARN:
        return spdlog::level::warn;
    case LogLevel::ERROR:
        return spdlog::level::err;
    case LogLevel::CRITICAL:
        return spdlog::level::critical;
    default:
        return spdlog::level::info;
    }
}

LogLevel Logger::ParseLevel(const char *s)
{
    if (!s)
        return LogLevel::INFO;
    if (strcmp(s, "TRACE") == 0)
        return LogLevel::TRACE;
    if (strcmp(s, "DEBUG") == 0)
        return LogLevel::DEBUG;
    if (strcmp(s, "INFO") == 0)
        return LogLevel::INFO;
    if (strcmp(s, "WARN") == 0 || strcmp(s, "WARNING") == 0)
        return LogLevel::WARN;
    if (strcmp(s, "ERROR") == 0)
        return LogLevel::ERROR;
    if (strcmp(s, "CRITICAL") == 0)
        return LogLevel::CRITICAL;
    return LogLevel::INFO;
}

// ─── LoggerImpl ──────────────────────────────────────────────────────────────

class LoggerImpl
{
  public:
    LoggerImpl() = default;
    ~LoggerImpl()
    {
        Shutdown();
    }

    int32_t Init(const std::string &process_name, const std::unordered_map<std::string, LogConf> &confs)
    {
        process_name_ = process_name;

        const LogConf *found = FindConf(process_name_, confs);
        if (found)
        {
            current_conf_ = *found;
            current_conf_.name = process_name_;
            if (current_conf_.filename.empty())
                current_conf_.filename = process_name_ + ".log";

            default_logger_ = CreateLogger(current_conf_);
            if (default_logger_)
            {
                spdlog::set_default_logger(default_logger_);
                return 0;
            }
        }

        // console fallback
        auto lg = spdlog::stdout_color_mt(process_name_);
        lg->set_level(spdlog::level::info);
        lg->set_pattern(kDefaultPattern);
        default_logger_ = lg;
        spdlog::set_default_logger(default_logger_);
        return 0;
    }

    void UpdateConf(const std::unordered_map<std::string, LogConf> &confs)
    {
        const LogConf *found = FindConf(process_name_, confs);
        if (!found || !default_logger_)
            return;

        LogConf nc = *found;
        nc.name = process_name_;
        nc.filename = current_conf_.filename; // filename never hot-reloaded

        bool need_recreate = (nc.async != current_conf_.async) || (nc.file_max_kb != current_conf_.file_max_kb) ||
                             (nc.file_max_no != current_conf_.file_max_no) ||
                             (nc.save_days != current_conf_.save_days) ||
                             (nc.log_on_monitor != current_conf_.log_on_monitor);

        if (need_recreate)
        {
            default_logger_->flush();
            auto new_lg = CreateLogger(nc);
            if (new_lg)
            {
                default_logger_ = new_lg;
                spdlog::set_default_logger(default_logger_);
                current_conf_ = nc;
                spdlog::info("Logger recreated: {}", process_name_);
            }
            return;
        }

        auto new_level = ToSpdlogLevel(nc.log_level);
        if (default_logger_->level() != new_level)
        {
            default_logger_->set_level(new_level);
            current_conf_.log_level = nc.log_level;
            spdlog::info("Log level updated -> {}", LogLevelToStr(nc.log_level));
        }

        if (nc.pattern != current_conf_.pattern)
        {
            default_logger_->set_pattern(nc.pattern.empty() ? kDefaultPattern : nc.pattern);
            current_conf_.pattern = nc.pattern;
            spdlog::info("Log pattern updated");
        }
    }

    void Shutdown()
    {
        if (default_logger_)
            default_logger_->flush();
        default_logger_.reset();
        thread_pool_.reset();
        spdlog::shutdown();
    }

    void Log(LogLevel lv, const char *file, const char *func, int line, const char *fmt, va_list args)
    {
        if (!default_logger_)
            return;
        char buf[4096];
        vsnprintf(buf, sizeof(buf), fmt, args);
        const char* slash = strrchr(file, '/');
        const char* basename = slash ? slash + 1 : file;
        default_logger_->log(ToSpdlogLevel(lv), "[{}:{}:{}] {}", basename, line, func, buf);
    }

  private:
    const LogConf *FindConf(const std::string &name, const std::unordered_map<std::string, LogConf> &confs)
    {
        auto it = confs.find(name);
        if (it != confs.end())
            return &it->second;
        it = confs.find("General");
        if (it != confs.end())
            return &it->second;
        if (!confs.empty())
            return &confs.begin()->second;
        return nullptr;
    }

    std::shared_ptr<spdlog::logger> CreateLogger(const LogConf &conf)
    {
        try
        {
            std::vector<spdlog::sink_ptr> sinks;

            if (conf.log_on_monitor)
                sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

            spdlog::sink_ptr file_sink;
            if (conf.save_days > 0)
            {
                file_sink =
                    std::make_shared<spdlog::sinks::daily_file_sink_mt>(conf.filename, 0, 0, false, conf.save_days);
            }
            else if (conf.file_max_kb > 0)
            {
                file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
                    conf.filename, static_cast<size_t>(conf.file_max_kb) * 1024, conf.file_max_no);
            }
            if (file_sink)
                sinks.push_back(file_sink);

            if (sinks.empty())
                sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());

            std::shared_ptr<spdlog::logger> logger;
            if (conf.async)
            {
                thread_pool_ = std::make_shared<spdlog::details::thread_pool>(8192, 1);
                logger = std::make_shared<spdlog::async_logger>(conf.name, sinks.begin(), sinks.end(), thread_pool_,
                                                                spdlog::async_overflow_policy::block);
            }
            else
            {
                logger = std::make_shared<spdlog::logger>(conf.name, sinks.begin(), sinks.end());
            }

            logger->set_level(ToSpdlogLevel(conf.log_level));
            logger->set_pattern(conf.pattern.empty() ? kDefaultPattern : conf.pattern);
            logger->flush_on(spdlog::level::info);

            spdlog::info("Logger created: name={} file={} level={} async={}", conf.name, conf.filename,
                         LogLevelToStr(conf.log_level), conf.async);
            return logger;
        }
        catch (const spdlog::spdlog_ex &ex)
        {
            spdlog::error("Logger creation failed: {}", ex.what());
            return nullptr;
        }
    }

  private:
    std::shared_ptr<spdlog::logger> default_logger_;
    std::shared_ptr<spdlog::details::thread_pool> thread_pool_;
    std::string process_name_;
    LogConf current_conf_;
};

// ─── Logger ──────────────────────────────────────────────────────────────────

int32_t Logger::Init(const std::string &process_name, const std::string &conf_file)
{
    if (inited_.exchange(true))
        return 0;

    process_name_ = process_name;
    log_conf_file_ = conf_file;

    int32_t ret = InitLogConf();
    if (ret != 0)
    {
        inited_ = false;
        return ret;
    }

    struct stat st;
    if (stat(log_conf_file_.c_str(), &st) == 0)
        last_mtime_ = st.st_mtime;

    logger_impl_ = new LoggerImpl();
    ret = logger_impl_->Init(process_name_, log_confs_);
    if (ret != 0)
    {
        inited_ = false;
        return ret;
    }

    running_ = true;
    reload_thread_ = std::thread(&Logger::ReloadLoop, this);
    return 0;
}

int32_t Logger::InitLogConf()
{
    using namespace tinyxml2;

    XMLDocument doc;
    if (doc.LoadFile(log_conf_file_.c_str()) != XML_SUCCESS)
    {
        spdlog::error("Failed to load config: {}", log_conf_file_);
        return -1;
    }

    auto *root = doc.FirstChildElement("LogConf");
    if (!root)
    {
        spdlog::error("Missing <LogConf> element");
        return -1;
    }

    check_period_s_ = root->IntAttribute("ChkCfgMdfyPeriod", 10);

    for (auto *e = root->FirstChildElement(); e; e = e->NextSiblingElement())
    {
        LogConf conf;
        conf.name = e->Name();
        conf.log_level = ParseLevel(e->Attribute("CurrentLogLV"));
        conf.async = e->IntAttribute("Async", 1) > 0;
        conf.log_on_monitor = e->IntAttribute("LogOnMonitor", 0) > 0;
        conf.file_max_kb = e->IntAttribute("FileMaxKb", 1024 * 100);
        conf.file_max_no = e->IntAttribute("FileMaxNo", 3);
        conf.save_days = e->IntAttribute("SaveDays", 0);

        const char *fn = e->Attribute("FileName");
        conf.filename = fn ? fn : "";

        const char *pat = e->Attribute("Pattern");
        conf.pattern = pat ? pat : "";

        log_confs_[conf.name] = conf;
    }
    return 0;
}

int32_t Logger::Shutdown()
{
    running_ = false;
    if (reload_thread_.joinable())
        reload_thread_.join();
    if (logger_impl_)
    {
        logger_impl_->Shutdown();
        delete logger_impl_;
        logger_impl_ = nullptr;
    }
    log_confs_.clear();
    inited_ = false;
    return 0;
}

void Logger::LogWithMeta(LogLevel lv, const char *file, const char *func, int line, const char *fmt, ...)
{
    if (!inited_)
        return;
    va_list args;
    va_start(args, fmt);
    VLogWithMeta(lv, file, func, line, fmt, args);
    va_end(args);
}

void Logger::VLogWithMeta(LogLevel lv, const char *file, const char *func, int line, const char *fmt, va_list args)
{
    if (!inited_ || !logger_impl_)
        return;
    logger_impl_->Log(lv, file, func, line, fmt, args);
}

void Logger::ReloadLoop()
{
    int elapsed_ms = 0;
    while (running_)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed_ms += 100;
        if (elapsed_ms >= check_period_s_ * 1000)
        {
            elapsed_ms = 0;
            CheckAndReload();
        }
    }
}

void Logger::CheckAndReload()
{
    struct stat st;
    if (stat(log_conf_file_.c_str(), &st) != 0)
        return;
    if (st.st_mtime == last_mtime_)
        return;

    last_mtime_ = st.st_mtime;
    spdlog::info("Config changed, reloading: {}", log_conf_file_);

    auto old = log_confs_;
    log_confs_.clear();
    if (InitLogConf() != 0)
    {
        log_confs_ = std::move(old);
        spdlog::error("Config reload failed, keeping old config");
        return;
    }
    if (logger_impl_)
        logger_impl_->UpdateConf(log_confs_);
}

} // namespace dsflog
