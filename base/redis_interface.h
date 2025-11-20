#include <hiredis/hiredis.h>

#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

class RedisClient {
 public:
  RedisClient() : context_(nullptr), port_(0), timeout_ms_(2000) {}
  ~RedisClient() { Disconnect(); }

  // 连接 Redis
  bool Connect(const std::string& host, int port, int timeout_ms = 2000) {
    std::lock_guard<std::mutex> lk(mutex_);
    host_ = host;
    port_ = port;
    timeout_ms_ = timeout_ms;

    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    context_ = redisConnectWithTimeout(host.c_str(), port, timeout);
    if (context_ == nullptr || context_->err) {
      if (context_) {
        std::cerr << "Redis connect error: " << context_->errstr << std::endl;
        redisFree(context_);
        context_ = nullptr;
      } else {
        std::cerr << "Redis connect failed: can't allocate redis context\n";
      }
      return false;
    }
    return true;
  }

  // 重连
  bool Reconnect() {
    Disconnect();
    return Connect(host_, port_, timeout_ms_);
  }

  // 断开连接
  void Disconnect() {
    std::lock_guard<std::mutex> lk(mutex_);
    if (context_) {
      redisFree(context_);
      context_ = nullptr;
    }
  }

  bool IsConnected() const { return context_ != nullptr; }

  // 执行普通命令，例如 "SET key value" 或 "GET key"
  std::string Command(const std::string& cmd) {
    redisReply* reply = nullptr;
    {
      std::lock_guard<std::mutex> lk(mutex_);
      if (!context_) return "Not connected";
      reply = (redisReply*)redisCommand(context_, cmd.c_str());
    }

    if (!reply) {
      std::cerr << "Redis command failed, try reconnect: " << cmd << std::endl;
      Reconnect();  
      return "";
    }

    std::string result;
    if (reply->type == REDIS_REPLY_STRING)
      result = reply->str;
    else if (reply->type == REDIS_REPLY_STATUS)
      result = reply->str ? reply->str : "";
    else if (reply->type == REDIS_REPLY_INTEGER)
      result = std::to_string(reply->integer);
    else if (reply->type == REDIS_REPLY_NIL)
      result = "(nil)";
    else if (reply->type == REDIS_REPLY_ERROR)
      result = std::string("Error: ") + reply->str;

    freeReplyObject(reply);
    return result;
  }

  // Pipeline 执行多个命令
  bool Pipeline(const std::vector<std::string>& cmds,
                std::vector<std::string>* replies) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (!context_) return false;

    // 添加到 pipeline
    for (const auto& cmd : cmds) {
      if (redisAppendCommand(context_, cmd.c_str()) != REDIS_OK) {
        std::cerr << "redisAppendCommand failed: " << cmd << std::endl;
        return false;
      }
    }

    // 获取返回结果
    for (size_t i = 0; i < cmds.size(); ++i) {
      redisReply* reply = nullptr;
      if (redisGetReply(context_, (void**)&reply) != REDIS_OK || !reply) {
        std::cerr << "redisGetReply failed at index " << i << std::endl;
        Reconnect();
        return false;
      }

      if (replies) {
        std::string result;
        if (reply->type == REDIS_REPLY_STRING) {
          result = reply->str;
        } else if (reply->type == REDIS_REPLY_STATUS) {
          result = reply->str ? reply->str : "";
        } else if (reply->type == REDIS_REPLY_INTEGER) {
          result = std::to_string(reply->integer);
        } else if (reply->type == REDIS_REPLY_NIL) {
          result = "(nil)";
        } else if (reply->type == REDIS_REPLY_ERROR) {
          result = std::string("Error: ") + reply->str;
        }
        replies->push_back(result);
      }

      freeReplyObject(reply);
    }

    return true;
  }

 private:
  redisContext* context_;
  std::string host_;
  int port_;
  int timeout_ms_;
  mutable std::mutex mutex_;
};
