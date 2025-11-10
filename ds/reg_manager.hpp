#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

struct DataHeader
{
    int type = 0;
};

struct ObjectAttr
{
};

struct TagkeyInfo
{
    std::string key;
    ObjectAttr attr;
};
using TagkeyInfoPtr = std::shared_ptr<TagkeyInfo>;

struct BatchRegInfo
{
    int32_t fd = -1;
    uint32_t batchId = 0;
    DataHeader data_header;
    std::vector<std::string> members;

    // keys for query redis
    std::vector<TagkeyInfoPtr> tagkey_infos;
    std::set<std::string> tagkey_set;
};
using BatchRegInfoPtr = std::shared_ptr<BatchRegInfo>;

class RegManager
{
  public:
    std::string getUdtName(const std::string &tag_name)
    {
        return tag_name;
    }
    bool IsSingleArrayElement(const std::string &tag_name)
    {
        return false;
    }

  public:
    void DeleteRegInfo(const int32_t fd, const uint32_t batchId = 0)
    {
        std::lock_guard<std::mutex> lock(batch_reg_infos_mutex_);
        if (0 == batchId)
        {
            batch_reg_infos_.erase(fd);
        }
        else
        {
            batch_reg_infos_[fd].erase(batchId);
            if (batch_reg_infos_[fd].empty())
            {
                batch_reg_infos_.erase(fd);
            }
        }
    }
    void AddRegInfo(const int32_t fd, const uint32_t batchId, const DataHeader &head,
                    const std::vector<std::string> &item)
    {

        auto batch_reg_info = std::make_shared<BatchRegInfo>();
        batch_reg_info->fd = fd;
        batch_reg_info->batchId = batchId;
        batch_reg_info->data_header = head;
        batch_reg_info->members = item;

        UpdateRegInfo(batch_reg_info);

        { // lock and add
            std::lock_guard<std::mutex> lock(batch_reg_infos_mutex_);
            batch_reg_infos_[fd][batchId] = batch_reg_info;
        }
    }

    void UpdateRegInfo(std::shared_ptr<BatchRegInfo> &batch_reg_info)
    {
        const auto member_size = batch_reg_info->members.size();
        for (int i = 0; i < member_size; i++)
        {
            auto tagkey_info = std::make_shared<TagkeyInfo>();
            const auto &tag_name = batch_reg_info->members[i];

            std::string key_name = getUdtName(tag_name);
            if (IsSingleArrayElement(key_name))
            {
                key_name = key_name.substr(0, key_name.find('['));
            }
            tagkey_info->key = key_name;
            batch_reg_info->tagkey_set.insert(key_name);
            batch_reg_info->tagkey_infos.emplace_back(tagkey_info);
        }
    }

  private:
    std::unordered_map<int32_t, std::unordered_map<uint32_t, BatchRegInfoPtr>>
        batch_reg_infos_; // fd, batchId, BatchRegInfoPtr
    std::mutex batch_reg_infos_mutex_;
};
