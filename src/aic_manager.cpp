/**
 * Filename        aic_manager.cpp
 * Copyright       Shanghai Baosight Software Co., Ltd.
 * Description
 *
 * Author          wuzheqiang
 * Version         01/10/2025    wuzheqiang    Initial Version
 **************************************************************/

#include "aic_manager.h"

#include <atomic>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

struct SmpPath
{
    std::string type;
    std::string sub_path;
};
static std::vector<SmpPath> kSmpPath = {{std::string("pdi"), std::string("/input")},
                                        {std::string("tdf"), std::string("/input")},
                                        {std::string("result"), std::string("/output")},
                                        {std::string("review"), std::string("/output")}};
int32_t AicManager::Init(const std::string &config_json)
{
    config_json_ = config_json;
    if (0 != LoadConfig())
    {
        std::cout << "LoadConfig failed!" << std::endl;
        return -1;
    }

    for (const auto &item : kSmpPath)
    {
        if (0 != LoadJsonData(item.type, spm_data_path_ + item.sub_path + "/" + item.type))
        {
            std::cout << "LoadConfig failed!type = " << item.type << std::endl;
            return -1;
        }
    }

    for (const auto &item : spm_json_data_)
    {
        std::cout << item.first << ":" << item.second.size() << std::endl;
        for (const auto &file : item.second)
        {
            std::cout << file.first << std::endl;
        }
    }

    return 0;
}

int32_t AicManager::LoadConfig()
{
    std::ifstream configFile(config_json_);
    if (!configFile.is_open())
    {
        std::cout << "Open config file[" << config_json_ << "] failed!" << std::endl;
        return -1;
    }
    json jConfig;
    configFile >> jConfig;
    configFile.close();

    if (!jConfig.contains("spm_data_path"))
    {
        std::cout << "config json not contain [spm_data_path]" << std::endl;
        return -1;
    }
    else
    {
        spm_data_path_ = jConfig["spm_data_path"].get<std::string>();
    }

    return 0;
}
int32_t AicManager::LoadJsonData(const std::string &type, const std::string &path)
{
    try
    {

        for (const auto &entry : fs::directory_iterator(path))
        {
            // 如果是文件并且以 .json 结尾
            if (fs::is_regular_file(entry) && entry.path().extension() == ".json")
            {
                // 打开 JSON 文件
                std::ifstream file(entry.path());
                if (file.is_open())
                {
                    try
                    {
                        // 创建一个 shared_ptr 来管理 json 对象
                        auto jFile = std::make_shared<json>();
                        file >> *jFile; // 解析 JSON 文件

                        if (spm_json_data_.find(type) == spm_json_data_.end())
                        {
                            spm_json_data_.emplace(type, MapJsonPtr());
                        }
                        spm_json_data_[type][entry.path()] = jFile;
                    }
                    catch (const std::exception &e)
                    {
                        std::cerr << "Error parsing file " << entry.path() << ": " << e.what() << std::endl;
                        return -1;
                    }
                }
                else
                {
                    std::cerr << "Unable to open file: " << entry.path() << std::endl;
                    return -1;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error reading directory: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}

int32_t AicManager::DealTDFQuery(std::string &json_string)
{

    auto &json_file_map = spm_json_data_["tdf"];
    auto &json_file = json_file_map.begin()->second;
    std::cout << "tdf file name:" << json_file_map.begin()->first << std::endl;

    json_string = json_file->dump();
    return 0;
}

int32_t AicManager::DealPDIQuery(std::string &json_string)
{
    auto &json_file_map = spm_json_data_["pdi"];
    auto &json_file = json_file_map.begin()->second;
    std::cout << "pdi file name:" << json_file_map.begin()->first << std::endl;

    if (json_file->contains("chinese"))
    {
        json_file->erase("chinese");
    }
    if (json_file->contains("TIMESTAMP_RECV"))
    {
        (*json_file)["TIMESTAMP_RECV"] = getTimestamp();
    }

    json_string = json_file->dump();
    return 0;
}