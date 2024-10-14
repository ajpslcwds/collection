/**
 * Filename        fake_ds.cpp
 * Copyright       Shanghai Baosight Software Co., Ltd.
 * Description     simulate DataService
 *
 * Author          wuzheqiang
 * Version         09/20/2024	wuzheqiang	Initial Version
 **************************************************************/

#include "drsdk_common.h"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <set>
#include <thread>
#include <unordered_map>
#include <vector>

using boost::asio::ip::tcp;
using drsdk::DataHeader;
struct RegInfo
{
    std::vector<std::string> vecTagName;
    uint16 nInterval;
};
using MapRegInfo = std::unordered_map<int32, RegInfo>;

constexpr char ksDivideMark[] = "@@";
constexpr int32 TIMER_INTERVAL = 10;
class ClientSession : public std::enable_shared_from_this<ClientSession>
{
  public:
    ClientSession(boost::asio::io_context &io_context) : socket_(io_context), timer_(io_context)
    {
    }

    tcp::socket &socket()
    {
        return socket_;
    }

    void Start()
    {
        ReadMessage();
        StartPushTimer();
    }

    void SendMessage(char *sBuf, const size_t nLen)
    {
        auto self(shared_from_this());
        boost::asio::async_write(
            socket_, boost::asio::buffer(sBuf, nLen),
            [this, self](const boost::system::error_code &error, std::size_t /*bytes_transferred*/) {
                if (error)
                {
                    LOG_ERR << "Failed to send message: " << error.message() << std::endl;
                    // Handle disconnection here
                }
            });
    }

  private:
    void StartPushTimer()
    {
        timer_.expires_after(std::chrono::milliseconds(TIMER_INTERVAL)); // Set push interval
        timer_.async_wait([TIMER_INTERVAL, this](const boost::system::error_code &error) {
            static uint32 times = 0;
            if (!error)
            {
                ++times;
                for (auto &item : batch_data_)
                {
                    auto &batch_id = item.first;
                    auto &reg_info = item.second;
                    auto &interval = reg_info.nInterval;
                    if ((times * TIMER_INTERVAL) % interval < TIMER_INTERVAL)
                    {
                        char sBuf[1024] = {0};
                        int32 nBufLen = sizeof(sBuf);
                        static int32 nSeq = 0;
                        std::string strContent = "";
                        for (auto &strTagName : reg_info.vecTagName)
                        {
                            // get tag value from redis or other storage
                            // construct msg content
                            strContent += strTagName + std::string(ksDivideMark) + drsdk::CurrentTimestampMicro() +
                                          std::string(ksDivideMark);
                        }
                        DataHeader tHead;
                        tHead.nType = drsdk::SDK_REG_TAG_DATA;
                        tHead.nLen = strContent.size();
                        tHead.nSeq = ++nSeq;
                        tHead.nReserve1 = batch_id;
                        memcpy(sBuf, &tHead, sizeof(tHead));
                        memcpy(sBuf + sizeof(tHead), strContent.c_str(), tHead.nLen);
                        nBufLen = sizeof(DataHeader) + tHead.nLen;
                        LOG_INFO << "send data.batch_id:" << batch_id << ", strContent = " << strContent
                                 << ", nBufLen = " << nBufLen << std::endl;
                        SendMessage(sBuf, nBufLen);
                    }
                }

                StartPushTimer(); // Restart timer
            }
        });
    }
    void ReadMessage()
    {
        auto self(shared_from_this());
        memset(data_, 0, drsdk::BUFFER_MAX_LENGTH);
        socket_.async_read_some(boost::asio::buffer(data_, drsdk::BUFFER_MAX_LENGTH),
                                [this, self](const boost::system::error_code &error, std::size_t bytes_transferred) {
                                    if (!error)
                                    {
                                        LOG_INFO << "recv data. bytes_transferred = " << bytes_transferred << std::endl;
                                        DealRequest(data_, bytes_transferred);
                                        // Continue reading
                                        ReadMessage();
                                    }
                                    else
                                    {
                                        LOG_ERR << "Failed to read message: " << error.message() << std::endl;
                                        socket_.close();
                                        // Handle disconnection here
                                    }
                                });
    }

    void DealRequest(char *sBuf, const size_t nLen)
    {
        int32 nRet = 0;
        int32 nBufLen = nLen;
        DataHeader *pHead = (DataHeader *)sBuf;
        LOG_INFO << pHead->ToString() << std::endl;
        const char *pContent = sBuf + sizeof(DataHeader);
        std::vector<std::string> vecContent;
        drsdk::SplitString(std::string(pContent, pHead->nLen), std::string(ksDivideMark), &vecContent);
        if (0 == vecContent.size())
        {
            LOG_ERR << "DealRequest vecContent size is 0" << std::endl;
            return;
        }

        switch (pHead->nType)
        {
        case drsdk::SDK_READ_DATA: {
            LOG_INFO << "SDK_READ_DATA readed data: " << std::string(pContent, pHead->nLen) << std::endl;
            // query the value
            std::string strValue = "";
            for (int i = 0; i < vecContent.size(); i++)
            {
                strValue += vecContent[i] + "->" + drsdk::CurrentTimestampMicro() + std::string(ksDivideMark);
            }

            strncpy(sBuf + sizeof(DataHeader), strValue.c_str(), strValue.size());
            pHead->nLen = strValue.size();
            pHead->nFlag = 0; // 0: success, 1: failed
            nBufLen = sizeof(DataHeader) + pHead->nLen;
            LOG_INFO << "SDK_READ_DATA send length:" << pHead->nLen << ", data:" << strValue << std::endl;
            SendMessage(sBuf, nBufLen);

            break;
        }
        case drsdk::SDK_CONTROL_DATA:
        case drsdk::SDK_DUMP_DATA: {
            for (int i = 0; i < vecContent.size(); i += 2)
            {
                LOG_INFO << "SDK_CONTROL_DATA/SDK_DUMP_DATA tagname:" << vecContent[i] << " value:" << vecContent[i + 1]
                         << std::endl;
            }
            // send the value
            pHead->nLen = 0;  // have not content data
            pHead->nFlag = 0; // 0: success, 1: failed
            nBufLen = sizeof(DataHeader);
            SendMessage(sBuf, nBufLen);

            break;
        }
        case drsdk::SDK_REG_TAG: {
            uint16 &nBatchId = pHead->nReserve1;
            uint16 &nInterval = pHead->nReserve2;
            LOG_INFO << "SDK_REG_TAG batch_id:" << nBatchId << ", interval:" << nInterval
                     << ", tag size:" << vecContent.size() << std::endl;
            RegInfo tRegInfo{vecContent, nInterval};
            batch_data_.emplace(nBatchId, std::move(tRegInfo));

            // send the value
            pHead->nLen = 0;  // have not content data
            pHead->nFlag = 0; // 0: success, 1: failed
            nBufLen = sizeof(DataHeader);
            SendMessage(sBuf, nBufLen);
            break;
        }
        case drsdk::SDK_READ_ATTR: {
            std::string strObjectName = vecContent[0];
            LOG_INFO << "SDK_READ_ATTR readed data: " << strObjectName << std::endl;
            // query the value
            std::string strValue = "";
            drsdk::ObjectAttr objAttr[10];
            for (int i = 0; i < 10; i++)
            {
                std::string strName = strObjectName + std::string(".tag") + std::to_string(i);
                strncpy(objAttr[i].sName, strName.c_str(), strName.size());
                strncpy(objAttr[i].sAliasName, strName.c_str(), strName.size());
                objAttr[i].nType = drsdk::SDKAttrType::SDK_ATTR_TYPE_INT8;
                objAttr[i].nLen = 10 + i;
            }

            memcpy(sBuf + sizeof(DataHeader), objAttr, sizeof(drsdk::ObjectAttr) * 10);
            pHead->nLen = sizeof(drsdk::ObjectAttr) * 10;
            pHead->nFlag = 0; // 0: success, 1: failed
            nBufLen = sizeof(DataHeader) + pHead->nLen;
            LOG_INFO << "SDK_READ_DATA send length:" << pHead->nLen << ", data:" << strValue << std::endl;
            SendMessage(sBuf, nBufLen);

            break;
        }
        case drsdk::SDK_READ_ATTR_DATA: {
            std::string strObjectName = vecContent[0];
            LOG_INFO << "SDK_READ_ATTR_DATA readed data: " << strObjectName << std::endl;
            // query the value
            std::string strValue = "";
            for (int i = 0; i < 10; i++)
            {
                strValue += strObjectName + std::string(".tag") + std::to_string(i) + std::string(ksDivideMark) +
                            drsdk::CurrentTimestampMicro() + std::string(ksDivideMark);
            }

            strncpy(sBuf + sizeof(DataHeader), strValue.c_str(), strValue.size());
            pHead->nLen = strValue.size();
            pHead->nFlag = 0; // 0: success, 1: failed
            nBufLen = sizeof(DataHeader) + pHead->nLen;
            LOG_INFO << "SDK_READ_ATTR_DATA send length:" << pHead->nLen << ", data:" << strValue << std::endl;
            SendMessage(sBuf, nBufLen);

            break;
        }
        default:
            LOG_INFO << "Not supported pHead->nType: " << static_cast<u_int32_t>(pHead->nType) << std::endl;
            break;
        }
    }

    tcp::socket socket_;
    char data_[drsdk::RECV_BUFFER_MAX_LENGTH]; // Buffer for async read operations
    std::map<uint16, RegInfo> batch_data_;
    boost::asio::steady_timer timer_;
};

class DataService
{
  public:
    DataService(boost::asio::io_context &io_context, const tcp::endpoint &endpoint)
        : acceptor_(io_context, endpoint), io_context_(io_context)
    {
        StartAccept();
    }

    void StartAccept()
    {
        auto new_session = std::make_shared<ClientSession>(io_context_);
        acceptor_.async_accept(new_session->socket(), [this, new_session](const boost::system::error_code &error) {
            if (!error)
            {
                LOG_INFO << "New client connected" << std::endl;
                new_session->Start();
            }
            StartAccept(); // Accept next connection
        });
    }

  private:
    tcp::acceptor acceptor_;
    boost::asio::io_context &io_context_;
};

int main()
{
    try
    {
        boost::asio::io_context io_context;

        tcp::endpoint endpoint(tcp::v4(), 1234);
        DataService server(io_context, endpoint);

        // Run the io_context to process asynchronous events
        io_context.run();
    }
    catch (std::exception &e)
    {
        LOG_ERR << "Exception: " << e.what() << std::endl;
    }

    return 0;
}
