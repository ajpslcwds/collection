#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

struct KeyInfo
{
    std::string name = "";
    std::multiset<uint32_t> intervals;
    uint32_t min_interval = 0;
};

struct RedisData
{
    std::string name = "";
    std::vector<uint8_t> data;
};

class QueryRedis
{
  public:
    QueryRedis() = default;
    ~QueryRedis()
    {
        Stop();
    }

    void Stop()
    {
        is_stop_.store(true);
        is_init_.store(false);
        key_infos_.clear();

        if (gen_task_thread_ && gen_task_thread_->joinable())
        {
            gen_task_thread_->join();
            gen_task_thread_ = nullptr;
        }
    }

    int Init()
    {
        if (is_init_.exchange(true))
        {
            return 0;
        }
        is_stop_.store(false);
        gen_task_thread_.reset(new std::thread(&QueryRedis::GenTask, this));

        return 0;
    }

    int AddData(const std::string &name, const uint32_t interval)
    {
        std::lock_guard<std::mutex> lock(key_mtx_);
        if (key_infos_.find(name) == key_infos_.end())
        {
            auto key_info = std::make_shared<KeyInfo>();
            key_info->name = name;
            key_info->intervals.insert(interval);
            key_info->min_interval = interval;

            key_infos_.emplace(name, key_info);
        }
        else
        {
            auto &key_info = key_infos_[name];
            key_info->intervals.insert(interval);
            key_info->min_interval = *(key_info->intervals.begin());
        }
        return 0;
    }

    int DeleteData(const std::string &name, const uint32_t interval)
    {
        std::lock_guard<std::mutex> lock(key_mtx_);
        if (key_infos_.find(name) != key_infos_.end())
        {
            auto &key_info = key_infos_[name];
            auto it = key_info->intervals.find(interval);
            if (it != key_info->intervals.end())
            {
                key_info->intervals.erase(it); // delete one item
            }
            if (key_info->intervals.empty())
            {
                key_infos_.erase(name);
            }
            else
            {
                key_info->min_interval = *(key_info->intervals.begin());
            }
        }
        return 0;
    }
    int QueryInterval(const std::string &name, uint32_t &interval)
    {
        std::lock_guard<std::mutex> lock(key_mtx_);
        if (key_infos_.find(name) != key_infos_.end())
        {
            interval = key_infos_[name]->min_interval;
            std::cout << "interval: " << interval << std::endl;
            return 0;
        }
        return -1;
    }

  private:
    void UpdateQueryData()
    {
        query_datas_.clear();
        std::lock_guard<std::mutex> lock(key_mtx_);
        for (auto &kv : key_infos_)
        {
            query_datas_[kv.second->min_interval].push_back(kv.first);
        }
    }

    void DisplayQueryData()
    {
        for (auto &kv : query_datas_)
        {
            std::cout << "interval: " << kv.first << " ";
            for (auto &name : kv.second)
            {
                std::cout << name << " ";
            }
            std::cout << std::endl;
        }
    }

    void GenTask()
    {

        while (!is_stop_)
        {

            UpdateQueryData();
            DisplayQueryData();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

  private:
    std::atomic_bool is_stop_ = {true};
    std::atomic_bool is_init_ = {false};

    std::mutex key_mtx_;
    std::unordered_map<std::string, std::shared_ptr<KeyInfo>> key_infos_; // { key, {key_info} }
    std::atomic_flag keys_chanages_flag_ = ATOMIC_FLAG_INIT;
    std::mutex query_mtx_; // TODO(wuzheqiang): 需要一个读写锁，保证query_datas_的读写安全
    std::unordered_map<uint32_t, std::vector<std::string>> query_datas_; // { interval, {name} }
    std::mutex redis_mtx_;
    std::unordered_map<std::string, std::shared_ptr<RedisData>> redis_datas_; // { key, {redis_data} }

    std::unique_ptr<std::thread> gen_task_thread_ = nullptr;
};