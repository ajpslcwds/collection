#ifndef DRSDK_MANAGER_H
#define DRSDK_MANAGER_H
#include "drsdk_common.h"
#include "drsdk_socket.h"
#include "thread_pool.h"
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace drsdk
{
using Callback = std::function<void(const std::string &)>;
using VecFuncs = std::vector<Callback>;
using MapFuncs = std::unordered_map<std::string, VecFuncs>;
using Promise = std::promise<std::string>;
using SharedPromise = std::shared_ptr<Promise>;
using SharedFuture = std::shared_future<std::string>;

struct DRSdkConfig
{
    std::string ip_addr;
    // uint16_t port;
    std::string port;
};

class DRSdkManager
{
  public:
    ~DRSdkManager();
    uint32_t Init();
    static DRSdkManager *Instance();

    int32_t DrGetId(const std::string &tag_name, uint32_t *value);
    int32_t DrControlData(const std::string &tag_name, const std::string &value, const uint8_t write_mode);
    int32_t DrReadData(const std::string &tag_name, std::string *value);
    int32_t DrAsyncRegTag(const std::string &tag_name, const Callback &call_back);
    int32_t DrAsyncRegTags(const std::vector<std::string> &tag_names, const Callback &call_back);

  private:
    DRSdkManager() = default;
    DRSdkManager(const DRSdkManager &) = delete;
    DRSdkManager &operator=(const DRSdkManager &) = delete;

    void ConnectToServer();
    void WriteToSendBuffer(const DataHeader &head, const std::string &content, const size_t len);

  private:
    DRSdkConfig config_;
    std::unique_ptr<DRSdkSocket> sdk_socket_ = nullptr;
    boost::asio::io_context io_context_;
    std::unique_ptr<std::thread> context_thread_ = nullptr;

    std::atomic<bool> is_connected_ = {false};
    std::atomic<uint32_t> head_seq_ = {0};
    std::unordered_map<uint32_t, std::tuple<SharedPromise, SharedFuture, uint32_t>> requests_;
    std::unique_ptr<char[]> recv_buffer_ = nullptr;
    std::unique_ptr<char[]> send_buffer_ = nullptr;
    std::atomic<int32_t> send_buffer_len_ = {0};
    std::mutex recv_buffer_mtx_;
    std::mutex send_buffer_mtx_;

    // call_back
    std::unique_ptr<ThreadPool> thread_pool_;
    std::unordered_map<std::string, uint32_t> id_map_;
    MapFuncs map_funcs_;
    std::mutex map_funcs_mtx_;

    static DRSdkManager *instance_;
    static std::mutex mtx_;
};

} // namespace drsdk
#endif