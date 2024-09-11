#include "drsdk_manager.h"

namespace drsdk
{
namespace
{
constexpr uint32_t BUFFER_MAX_LENGTH = 1024;
constexpr uint32_t SEND_BUFFER_MAX_LENGTH = BUFFER_MAX_LENGTH;
constexpr uint32_t RECV_BUFFER_MAX_LENGTH = BUFFER_MAX_LENGTH;
} // namespace

DRSdkManager *DRSdkManager::instance_ = nullptr;
std::mutex DRSdkManager::mtx_;

DRSdkManager::~DRSdkManager()
{
    if (context_thread_)
    {
        io_context_.stop();
        context_thread_->join();
    }
}

DRSdkManager *DRSdkManager::Instance()
{
    if (nullptr == instance_)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (nullptr == instance_)
        {
            instance_ = new DRSdkManager();
            instance_->Init();
        }
    }
    return instance_;
}

uint32_t DRSdkManager::Init()
{
    LOG_INFO << " DRSdkManager::Init()" << std::endl;
    config_.ip_addr = "127.0.0.1";
    config_.port = "12345";

    boost::system::error_code ec;
    sdk_socket_ = std::make_unique<DRSdkSocket>(io_context_);

    ConnectToServer();
    context_thread_.reset(new std::thread([this]() { io_context_.run(); }));

    send_buffer_.reset(new char[SEND_BUFFER_MAX_LENGTH]);
    recv_buffer_.reset(new char[RECV_BUFFER_MAX_LENGTH]);

    // thread_pool_ = std::make_unique<ThreadPool>(4);

    return 0;
}

void DRSdkManager::ConnectToServer()
{
    if (is_connected_.load())
    {
        sdk_socket_->Close();
        is_connected_.store(false);
    }

    sdk_socket_->AsyncConnect(config_.ip_addr, config_.port, [this](boost::system::error_code ec) {
        if (!ec)
        {
            LOG_INFO << "Connected to server!" << std::endl;
            is_connected_.store(true);
            sdk_socket_->AsyncRead([this](const std::string &res, const boost::system::error_code &ec) {
                if (!ec)
                {

                    LOG_INFO << "Received data: " << res << std::endl;
                }
                else
                {
                    LOG_ERR << "Read failed: " << ec.message() << std::endl;
                    is_connected_.store(false);
                }
            });
        }
        else
        {
            LOG_ERR << "Connection failed: " << ec.message() << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(1));
            boost::system::error_code ec;
            ConnectToServer();
        }
    });
}

int32_t DRSdkManager::DrGetId(const std::string &tag_name, uint32_t *value)
{
    // static int cnt = 0;
    // if (!is_connected_.load() && cnt++ > 2)
    // {
    //     ConnectToServer();
    //     return -1;
    // }
    auto iter = id_map_.find(tag_name);
    if (iter != id_map_.end())
    {
        *value = iter->second;
        return 0;
    }

    DataHeader head;
    head.type = SDK_GET_ID;
    head.len = tag_name.size();
    head.seq = head_seq_.fetch_add(1);
    WriteToSendBuffer(head, tag_name, tag_name.size());

    auto tuple = requests_[head.seq];

    auto future = std::get<1>(tuple);
    auto status = future.wait_for(std::chrono::seconds(2));
    if (status != std::future_status::ready)
    {
        LOG_INFO << "time_out: " << std::endl;
        return -1;
    }
    std::string res = future.get();
    LOG_INFO << "res: " << res << std::endl;
    *value = static_cast<uint32_t>(std::stoul(res));

    return 0;
}

int32_t DRSdkManager::DrControlData(const std::string &tag_name, const std::string &value, const uint8_t write_mode)
{
    boost::system::error_code ec;
    char buf[1024] = {0};
    DataHeader *head = (DataHeader *)buf;
    head->type = SDK_CONTROL_DATA;
    head->len = tag_name.size();

    memcpy(buf + sizeof(DataHeader), tag_name.c_str(), tag_name.size());
    size_t len = sizeof(DataHeader) + tag_name.size();
    sdk_socket_->Write(buf, len, ec);
    if (ec)
    {
        LOG_ERR << "Error sending msg: " << ec.message() << std::endl;
        return -1;
    }
    std::string res = sdk_socket_->Read(ec);
    if (ec)
    {
        LOG_ERR << "Error receiving: " << ec.message() << std::endl;
        return -1;
    }

    LOG_INFO << "res: " << res << std::endl;
    return 0;
}

int32_t DRSdkManager::DrReadData(const std::string &tag_name, std::string *value)
{
    boost::system::error_code ec;
    char buf[1024] = {0};
    DataHeader *head = (DataHeader *)buf;
    head->type = SDK_READ_DATA;
    head->len = tag_name.size();

    memcpy(buf + sizeof(DataHeader), tag_name.c_str(), tag_name.size());
    size_t len = sizeof(DataHeader) + tag_name.size();
    sdk_socket_->Write(buf, len, ec);
    if (ec)
    {
        LOG_ERR << "Error sending msg: " << ec.message() << std::endl;
        return -1;
    }
    std::string res = sdk_socket_->Read(ec);
    if (ec)
    {
        LOG_ERR << "Error receiving: " << ec.message() << std::endl;
        return -1;
    }

    LOG_INFO << "res: " << res << std::endl;
    *value = std::move(res);

    return 0;
}

int32_t DRSdkManager::DrAsyncRegTag(const std::string &tag_name, const Callback &call_back)
{
    {
        std::lock_guard<std::mutex> lock(map_funcs_mtx_);
        auto iter = map_funcs_.find(tag_name);
        if (iter != map_funcs_.end())
        {
            auto &vec_funcs = iter->second;
            vec_funcs.emplace_back(call_back);
        }
        else
        {
            std::vector<Callback> vec_funcs;
            vec_funcs.emplace_back(call_back);
            map_funcs_.emplace(tag_name, vec_funcs);
        }
    }

    boost::system::error_code ec;
    char buf[1024] = {0};
    DataHeader *head = (DataHeader *)buf;
    head->type = SDK_REG_TAG;
    head->len = tag_name.size();

    memcpy(buf + sizeof(DataHeader), tag_name.c_str(), tag_name.size());
    size_t len = sizeof(DataHeader) + tag_name.size();
    sdk_socket_->Write(buf, len, ec);
    if (ec)
    {
        LOG_ERR << "Error sending msg: " << ec.message() << std::endl;
        return -1;
    }
    std::string res = sdk_socket_->Read(ec);
    if (ec)
    {
        LOG_ERR << "Error receiving: " << ec.message() << std::endl;
        return -1;
    }

    LOG_INFO << "res: " << res << std::endl;
    return 0;
}

int32_t DRSdkManager::DrAsyncRegTags(const std::vector<std::string> &tag_names, const Callback &call_back)
{
    return 0;
}

void DRSdkManager::WriteToSendBuffer(const DataHeader &head, const std::string &content, const size_t len)
{

    if (sizeof(head) + len > SEND_BUFFER_MAX_LENGTH - send_buffer_len_)
    {
        LOG_ERR << "send buffer is full. SEND_BUFFER_MAX_LENGTH:" << SEND_BUFFER_MAX_LENGTH << "send_buffer_len_"
                << send_buffer_len_;
        return;
    }

    {
        std::lock_guard<std::mutex> lock(send_buffer_mtx_);
        memcpy(send_buffer_.get() + send_buffer_len_, &head, sizeof(head));
        memcpy(send_buffer_.get() + send_buffer_len_ + sizeof(head), content.c_str(), len);
        send_buffer_len_ += sizeof(head) + len;
    }

    SharedPromise call_promise = std::make_shared<Promise>();
    SharedFuture f(call_promise->get_future());
    requests_[head.seq] = std::make_tuple(call_promise, f, NowSecond());
}
} // namespace drsdk