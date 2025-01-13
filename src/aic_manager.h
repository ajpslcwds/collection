/**
 * Filename        aic_manager.h
 * Copyright       Shanghai Baosight Software Co., Ltd.
 * Description
 *
 * Author          wuzheqiang
 * Version         01/10/2025    wuzheqiang    Initial Version
 **************************************************************/

#ifndef AIC_MANAGER_H
#define AIC_MANAGER_H

#include "nlohmann/json.hpp"
#include "stdint.h"
#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using json = nlohmann::json;
using JsonPtr = std::shared_ptr<json>;
using MapJsonPtr = std::unordered_map<std::string, JsonPtr>;

inline std::string getTimestamp()
{
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream timestamp;
    std::tm tm;
    localtime_r(&time, &tm);

    timestamp << std::put_time(&tm, "%Y%m%d-%H%M%S-") << std::setw(3) << std::setfill('0') << milliseconds.count();
    return timestamp.str();
}

class AicManager
{
  public:
    ~AicManager() = default;

    static AicManager &GetInstance()
    {
        static AicManager instance;
        return instance;
    }

    int32_t Init(const std::string &config_json);
    int32_t LoadConfig();
    int32_t LoadJsonData(const std::string &type, const std::string &path);

    int32_t DealTDFQuery(std::string &json_string);
    int32_t DealPDIQuery(std::string &json_string);

  private:
    AicManager() = default;

  private:
    std::string config_json_ = "";
    std::string spm_data_path_ = "";

    std::unordered_map<std::string, MapJsonPtr> spm_json_data_;
};
#endif