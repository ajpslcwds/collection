/*  Filename:    drsdk_manager.h
 *  Copyright:   Shanghai Baosight Software Co., Ltd.
 *
 *  Description: Define DRSdkManager, It is a singleton, managing the processing of data
 *
 *  @author:     wuzheqiang
 *  @version	   09/10/2024	wuzheqiang	Initial Version
 **************************************************************/

#ifndef DRSDK_MANAGER_H
#define DRSDK_MANAGER_H
#include "drsdk_common.h"
#include "drsdk_transport_socket.h"
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
// using Callback = std::function<void(const std::string &strTagName, const std::string &strValue)>;
// using VecFuncs = std::vector<Callback>;
// using MapFuncs = std::unordered_map<std::string, VecFuncs>;
using Callback = std::function<void(const std::map<std::string, std::string> &mapTagValue)>;
using Promise = std::promise<std::string>;
using SharedPromise = std::shared_ptr<Promise>;
using SharedFuture = std::shared_future<std::string>;
struct RegInfo
{
    std::vector<std::string> vecTagName;
    uint16 nInterval;
    Callback pCallback;
};
using MapRegInfo = std::unordered_map<int32, RegInfo>;
class DRSdkManager
{
  public:
    ~DRSdkManager();

    /**
     * @brief		  init DRSdkManager
     * @param [in]     strIp  dataservice ip
     * @param [in]     nPort  dataservice port
     * @param [in]     nTimeoutMs  socket timeout ms
     * @return		0 if success
     * @version		09/10/2024	wuzheqiang	Initial Version
     */
    int32 Init(const std::string &strIp, const int16 nPort, const uint16 nTimeoutMs);
    /**
     * @brief          uninit drsdk
     * @return         0 if success
     * @version        2024/09/25	wuzheqiang	Initial Version
     */
    int32 Uninit();

    /**
     * @brief          get connection status
     * @return         true if connected else false
     * @version        2024/10/09	wuzheqiang	Initial Version
     */
    bool DrConnectStatus();

    /**
     * @brief		  Get singleton instance
     * @return		DRSdkManager *m_pInstance
     * @version		09/10/2024	wuzheqiang	Initial Version
     */
    static DRSdkManager *Instance();

    /**
     * @brief		Send control commands
     * @param [in]	vecTagName  tag names
     * @param [in]	vecValue   tag values
     * @return		0 if success
     * @version		09/10/2024	wuzheqiang	Initial Version
     */
    int32 DrControlData(const std::vector<std::string> &vecTagName, const std::vector<std::string> &vecValue);

    /**
     * @brief          save data to storage
     * @param [in]     vecTagName  tag names
     * @param [out]    vecValue   tag values
     * @return         0 if success
     * @version        2024/09/29	wuzheqiang	Initial Version
     */
    int32 DrSaveData(const std::vector<std::string> &vecTagName, const std::vector<std::string> &vecValue);

    /**
     * @brief		Synchronize data reading
     * @param [in]	vecTagName  tag names
     * @param [out]	vecValue   tag values
     * @return		0 if success
     * @version		09/10/2024	wuzheqiang	Initial Version
     */
    int32 DrReadData(const std::vector<std::string> &vecTagName, std::vector<std::string> *vecValue);

    /**
     * @brief		register tag for async reading
     * @param [in]	vecTagName  tag names
     * @param [in]	nInterval   Interval for querying data, unit:ms
     * @param [in]	pCallBack   callback function ptr
     * @param [in]	nOldBatchId   batch id for automatic regeister; default 0 for new register
     * @return		0 if success
     * @version		09/10/2024	wuzheqiang	Initial Version
     */

    int32 DrAsyncRegTag(const std::vector<std::string> &vecTagName, const uint16 nInterval, const Callback &pCallBack,
                        const uint16 nOldBatchId = 0);

    /**
     * @brief          automatic  register after connectd
     * @version        2024/10/09	wuzheqiang	Initial Version
     */
    void DrAutomaticRegister();

    /**
     * @brief          get object attribute
     * @param [in]     objectName  object name
     * @param [out]    vecAttrs    object attributes
     * @return         0 if success
     * @version        2024/09/25	wuzheqiang	Initial Version
     */
    int32 DrReadObjectAttr(const std::string &strObjectName, std::vector<drsdk::ObjectAttr> *vecAttrs);

    /**
     * @brief          get object data
     * @param [in]     objectName  object name
     * @param [out]    vecAttrValues  object attribute  values
     * @return
     * @version        2024/09/25	wuzheqiang	Initial Version
     */
    int32 DrReadObectData(const std::string &strObjectName, std::vector<drsdk::ObjectData> *vecAttrValues);

  private:
    DRSdkManager() = default;
    DRSdkManager(const DRSdkManager &) = delete;
    DRSdkManager &operator=(const DRSdkManager &) = delete;

    /**
     * @brief		callback function that loops through to get data
     * @version		09/10/2024	wuzheqiang	Initial Version
     */
    void RecvThreadCallback();

    /**
     * @brief		Write content into  send_buffer
     * @param [in]	tHead   msg head
     * @param [in]	strContent   msg content
     * @param [in]	nLen  msg length
     * @return		0 if success
     * @version		09/10/2024	wuzheqiang	Initial Version
     */

    int32 WriteToSendBuffer(const DataHeader &tHead, const std::string &strContent, const size_t nLen);
    /**
     * @brief          async get response result
     * @param [in]     tHead   msg head
     * @param [out]    pRes    response result
     * @return         0 if success
     * @version        2024/09/23	wuzheqiang	Initial Version
     */
    int32 AsyncGetResult(const DataHeader &tHead, std::string *pRes);

  private:
    std::atomic_bool m_bStop = {false};
    DRSdkConfig m_tDrSdkConfig;
    std::unique_ptr<DrSdkTransportSocket> m_pTransport = nullptr; // transport layer
    std::unique_ptr<std::thread> m_pRecvThread = nullptr;         // recv from DS thread

    std::atomic<uint32> m_nHeadSeq = {0}; // request head seq
    std::mutex m_RequestMtx;
    std::unordered_map<uint32, std::tuple<SharedPromise, SharedFuture, uint32>> m_mapRequest; // async requests
    std::unique_ptr<char[]> m_pRecvBuffer = nullptr;
    std::unique_ptr<char[]> m_pSendBuffer = nullptr;
    std::atomic<int32> m_nSendBufferLen = {0};
    std::mutex m_RecvBufferMtx;
    std::mutex m_SendBufferMtx;

    // call_back
    std::atomic<uint16> m_nBatchId = {1};      // start from 1 ;  0 is  invalid
    std::unique_ptr<ThreadPool> m_pThreadPool; // thread pool,for async exec register callback
    MapRegInfo m_mapRegInfos;                  // register info
    std::mutex m_CallbacksMtx;

    static DRSdkManager *m_pInstance; // singleton instance
    static std::mutex m_InstanceMtx;
};

} // namespace drsdk
#endif