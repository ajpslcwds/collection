/*  Filename:    drsdk_manager.h
 *  Copyright:   Shanghai Baosight Software Co., Ltd.
 *
 *  Description: Define DRSdkManager, It is a singleton, managing the processing of data
 *
 *  @author:     wuzheqiang
 *  @version:    09/10/2024	wuzheqiang	Initial Version
 **************************************************************/

#include "drsdk_manager.h"
namespace drsdk
{
namespace
{
constexpr uint32 MAX_THREAD_NUM = 10;       // thread number for thread_pool
constexpr uint32 WAITING_TIMEOUT_MS = 1000; // milliseconds for waiting
constexpr char ksDivideMark[] = "@@";
} // namespace

DRSdkManager *DRSdkManager::m_pInstance = nullptr;
std::mutex DRSdkManager::m_InstanceMtx;

DRSdkManager::~DRSdkManager()
{
    m_bStop.store(true);
    if (m_pRecvThread && m_pRecvThread->joinable())
    {
        m_pRecvThread->join();
        m_pRecvThread = nullptr;
    }
}
/**
 * @brief		Get singleton instance
 * @return		DRSdkManager *m_pInstance
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
DRSdkManager *DRSdkManager::Instance()
{
    if (nullptr == m_pInstance)
    {
        std::lock_guard<std::mutex> lock(m_InstanceMtx);
        if (nullptr == m_pInstance)
        {
            m_pInstance = new DRSdkManager();
        }
    }
    return m_pInstance;
}

/**
 * @brief		init DRSdkManager
 * @param [in]     strIp  dataservice ip
 * @param [in]     nPort  dataservice port
 * @param [in]     nTimeoutMs  socket timeout ms
 * @return		0 if success
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
int32 DRSdkManager::Init(const std::string &strIp, const int16 nPort, const uint16 nTimeoutMs)
{
    int32 nRet = 0;
    LOG_INFO << " DRSdkManager::Init()" << std::endl;
    m_tDrSdkConfig.strIp = strIp;
    m_tDrSdkConfig.nPort = nPort;
    // m_tDrSdkConfig.nTimeoutMs = WAITING_TIMEOUT_MS;
    m_tDrSdkConfig.nTimeoutMs = nTimeoutMs;
    m_tDrSdkConfig.nThreadPoolNum = MAX_THREAD_NUM;
    LOG_INFO << "m_tDrSdkConfig:" << m_tDrSdkConfig.ToString() << std::endl;

    m_pTransport = std::make_unique<DrSdkTransportSocket>();

    nRet = m_pTransport->Init(strIp, nPort, nTimeoutMs);
    if (0 != nRet)
    {
        LOG_ERR << "m_pTransport->Init() failed" << std::endl;
        //   return nRet;
    }
    m_pTransport->SetConnectedCallback(std::bind(&DRSdkManager::DrAutomaticRegister, this));

    m_pSendBuffer.reset(new char[SEND_BUFFER_MAX_LENGTH]);
    m_pRecvBuffer.reset(new char[RECV_BUFFER_MAX_LENGTH]);
    if (!m_pSendBuffer || !m_pRecvBuffer)
    {
        LOG_ERR << "m_pSendBuffer or m_pRecvBuffer is null" << std::endl;
        return EC_SDK_MALLOC_FAILED;
    }

    m_pRecvThread.reset(new std::thread(&DRSdkManager::RecvThreadCallback, this));
    pthread_t thread = m_pRecvThread->native_handle();
    pthread_setname_np(thread, "recv");
    m_pThreadPool = std::make_unique<ThreadPool>(MAX_THREAD_NUM);

    return EC_SDK_SUCCESS;
}

/**
 * @brief          uninit drsdk
 * @return         0 if success
 * @version        2024/09/25	wuzheqiang	Initial Version
 */
int32 DRSdkManager::Uninit()
{
    m_bStop.store(true);
    m_mapRegInfos.clear();
    if (nullptr != m_pTransport)
    {
        m_pTransport->Close();
    }
    return EC_SDK_SUCCESS;
}

/**
 * @brief          get connection status
 * @return         true if connected else false
 * @version        2024/10/09	wuzheqiang	Initial Version
 */
bool DRSdkManager::DrConnectStatus()
{
    if (nullptr != m_pTransport)
    {
        return m_pTransport->GetConnectStatus();
    }
    else
        return false;
}

/**
 * @brief		Send control commands
 * @param [in]	vecTagName  tag names
 * @param [in]	vecValue   tag values
 * @return		0 if success
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
int32 DRSdkManager::DrControlData(const std::vector<std::string> &vecTagName, const std::vector<std::string> &vecValue)
{
    std::string strContent = "";
    if (vecTagName.size() != vecValue.size())
    {
        LOG_ERR << "DrControlData vecTagName.size() != vecValue.size()." << std::endl;
        return EC_SDK_INCORRECT_INPUT_PARAMS;
    }
    for (int i = 0; i < vecTagName.size(); i++)
    {
        strContent += vecTagName[i] + std::string(ksDivideMark) + vecValue[i] + std::string(ksDivideMark);
    }
    DataHeader tHead;
    tHead.nType = SDK_CONTROL_DATA;
    tHead.nLen = strContent.size();
    tHead.nSeq = m_nHeadSeq.fetch_add(1);

    int32 nRet = WriteToSendBuffer(tHead, strContent, strContent.size());
    if (0 != nRet)
    {
        LOG_ERR << "WriteToSendBuffer failed. nRet = " << nRet << std::endl;
        return nRet;
    }
    strContent.clear();
    nRet = AsyncGetResult(tHead, &strContent);
    if (0 != nRet)
    {
        LOG_ERR << "AsyncGetResult failed. nRet = " << nRet << std::endl;
        return nRet;
    }
    DataHeader *pHead = reinterpret_cast<DataHeader *>(const_cast<char *>(strContent.c_str()));
    if (pHead->nFlag != 0)
    {
        LOG_ERR << "pHead->nFlag = " << static_cast<int32>(pHead->nFlag) << std::endl;
        return static_cast<int32>(pHead->nFlag);
    }
    return EC_SDK_SUCCESS;
}

/**
 * @brief          Save data to storage
 * @param [in]     vecTagName  tag names
 * @param [out]    vecValue   tag values
 * @return         0 if success
 * @version        2024/09/29	wuzheqiang	Initial Version
 */
int32 DRSdkManager::DrSaveData(const std::vector<std::string> &vecTagName, const std::vector<std::string> &vecValue)
{
    std::string strContent = "";
    if (vecTagName.size() != vecValue.size())
    {
        LOG_ERR << "DrControlData vecTagName.size() != vecValue.size()." << std::endl;
        return EC_SDK_INCORRECT_INPUT_PARAMS;
    }
    for (int i = 0; i < vecTagName.size(); i++)
    {
        strContent += vecTagName[i] + std::string(ksDivideMark) + vecValue[i] + std::string(ksDivideMark);
    }
    DataHeader tHead;
    tHead.nType = SDK_DUMP_DATA;
    tHead.nLen = strContent.size();
    tHead.nSeq = m_nHeadSeq.fetch_add(1);

    int32 nRet = WriteToSendBuffer(tHead, strContent, strContent.size());
    if (0 != nRet)
    {
        LOG_ERR << "WriteToSendBuffer failed. nRet = " << nRet << std::endl;
        return nRet;
    }
    strContent.clear();
    nRet = AsyncGetResult(tHead, &strContent);
    if (0 != nRet)
    {
        LOG_ERR << "AsyncGetResult failed. nRet = " << nRet << std::endl;
        return nRet;
    }
    DataHeader *pHead = reinterpret_cast<DataHeader *>(const_cast<char *>(strContent.c_str()));
    if (pHead->nFlag != 0)
    {
        LOG_ERR << "pHead->nFlag = " << static_cast<int32>(pHead->nFlag) << std::endl;
        return static_cast<int32>(pHead->nFlag);
    }
    return EC_SDK_SUCCESS;
}

/**
 * @brief		Synchronize data reading
 * @param [in]	vecTagName  tag names
 * @param [out]	vecValue   tag values
 * @return		0 if success
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
int32 DRSdkManager::DrReadData(const std::vector<std::string> &vecTagName, std::vector<std::string> *vecValue)
{
    auto TagLength = vecTagName.size();
    if (0 == TagLength)
    {
        LOG_ERR << "vecLength is 0." << std::endl;
        return EC_SDK_INCORRECT_INPUT_PARAMS;
    }
    std::string strContent = "";
    for (int i = 0; i < vecTagName.size(); i++)
    {
        strContent += vecTagName[i] + std::string(ksDivideMark);
    }

    DataHeader tHead;
    tHead.nType = SDK_READ_DATA;
    tHead.nLen = strContent.size();
    tHead.nSeq = m_nHeadSeq.fetch_add(1);

    int32 nRet = WriteToSendBuffer(tHead, strContent, strContent.size());
    if (0 != nRet)
    {
        LOG_ERR << "WriteToSendBuffer failed. nRet = " << nRet << std::endl;
        return nRet;
    }
    strContent.clear();
    nRet = AsyncGetResult(tHead, &strContent);
    if (0 != nRet)
    {
        LOG_ERR << "AsyncGetResult failed. nRet = " << nRet << std::endl;
        return nRet;
    }
    DataHeader *pHead = reinterpret_cast<DataHeader *>(const_cast<char *>(strContent.c_str()));
    if (pHead->nFlag != 0)
    {
        LOG_ERR << "pHead->nFlag = " << static_cast<int32>(pHead->nFlag) << std::endl;
        return static_cast<int32>(pHead->nFlag);
    }
    SplitString(strContent.substr(sizeof(DataHeader), pHead->nLen), ksDivideMark, vecValue);
    auto nValueLength = vecValue->size();
    LOG_INFO << " DrReadData TagLength:" << TagLength << ", nValueLength:" << nValueLength << std::endl;
    return TagLength == nValueLength ? EC_SDK_SUCCESS : EC_SDK_COMMON_ERROR;
}

/**
 * @brief		register tag for async reading
 * @param [in]	vecTagName  tag names
 * @param [in]	nInterval   Interval for querying data, unit:ms ,range:[0-65535] ms
 * @param [in]	pCallback   callback function ptr
 * @param [in]	nOldBatchId   batch id for automatic regeister; default 0 for new register
 * @return		0 if success
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
int32 DRSdkManager::DrAsyncRegTag(const std::vector<std::string> &vecTagName, const uint16 nInterval,
                                  const Callback &pCallBack, const uint16 nOldBatchId)
{
    if (0 == vecTagName.size())
    {
        LOG_ERR << "vecLength is 0." << std::endl;
        return EC_SDK_INCORRECT_INPUT_PARAMS;
    }
    uint16 nBatchId = 0;
    if (nOldBatchId != 0)
    { // automatic regeister
        nBatchId = nOldBatchId;
    }
    else
    { // should record
        nBatchId = m_nBatchId.fetch_add(1);
        RegInfo regInfo{vecTagName, nInterval, pCallBack};
        {
            std::lock_guard<std::mutex> lock(m_CallbacksMtx);
            m_mapRegInfos.emplace(nBatchId, std::move(regInfo));
        }
    }

    std::string strContent = "";

    for (auto &strTagName : vecTagName)
    {
        strContent += strTagName + std::string(ksDivideMark);
    }

    DataHeader tHead;
    tHead.nType = SDK_REG_TAG;
    tHead.nLen = strContent.size();
    tHead.nSeq = m_nHeadSeq.fetch_add(1);
    tHead.nReserve1 = nBatchId;
    tHead.nReserve2 = nInterval;
    int32 nRet = WriteToSendBuffer(tHead, strContent, strContent.size());
    if (0 != nRet)
    {
        LOG_ERR << "WriteToSendBuffer failed. nRet = " << nRet << std::endl;
        return nRet;
    }
    LOG_INFO << "DrAsyncRegTag send tHead: " << tHead.ToString() << ", content: " << strContent << std::endl;

    strContent.clear();
    nRet = AsyncGetResult(tHead, &strContent);
    if (0 != nRet)
    {
        LOG_ERR << "AsyncGetResult failed. nRet = " << nRet << std::endl;
        return nRet;
    }
    DataHeader *pHead = reinterpret_cast<DataHeader *>(const_cast<char *>(strContent.c_str()));
    if (pHead->nFlag != 0)
    {
        LOG_ERR << "pHead->nFlag = " << static_cast<int32>(pHead->nFlag) << std::endl;
        return static_cast<int32>(pHead->nFlag);
    }
    return EC_SDK_SUCCESS;
}

/**
 * @brief          automatic register after connectd
 * @version        2024/10/09	wuzheqiang	Initial Version
 */
void DRSdkManager::DrAutomaticRegister()
{
    std::lock_guard<std::mutex> lock(m_CallbacksMtx);
    for (auto &item : m_mapRegInfos)
    {
        DrAsyncRegTag(item.second.vecTagName, item.second.nInterval, item.second.pCallback, item.first);
    }
}

/**
 * @brief          get object attribute
 * @param [in]     objectName  object name
 * @param [out]    vecAttrs    object attributes
 * @return         0 if success
 * @version        2024/09/25	wuzheqiang	Initial Version
 */
int32 DRSdkManager::DrReadObjectAttr(const std::string &strObjectName, std::vector<drsdk::ObjectAttr> *vecAttrs)
{
    auto nSize = strObjectName.size();
    if (0 == nSize)
    {
        LOG_ERR << "strObjectName size is 0." << std::endl;
        return EC_SDK_INCORRECT_INPUT_PARAMS;
    }

    std::string strContent = strObjectName;
    DataHeader tHead;
    tHead.nType = SDK_READ_ATTR;
    tHead.nLen = strContent.size();
    tHead.nSeq = m_nHeadSeq.fetch_add(1);

    int32 nRet = WriteToSendBuffer(tHead, strContent, strContent.size());
    if (0 != nRet)
    {
        LOG_ERR << "WriteToSendBuffer failed. nRet = " << nRet << std::endl;
        return nRet;
    }
    strContent.clear();
    nRet = AsyncGetResult(tHead, &strContent);
    if (0 != nRet)
    {
        LOG_ERR << "AsyncGetResult failed. nRet = " << nRet << std::endl;
        return nRet;
    }
    DataHeader *pHead = reinterpret_cast<DataHeader *>(const_cast<char *>(strContent.c_str()));
    if (pHead->nFlag != 0)
    {
        LOG_ERR << "pHead->nFlag = " << static_cast<int32>(pHead->nFlag) << std::endl;
        return static_cast<int32>(pHead->nFlag);
    }
    // SplitString(strContent.substr(sizeof(DataHeader), pHead->nLen), ksDivideMark, vecAttrs);
    if (0 == pHead->nLen || 0 != pHead->nLen % sizeof(ObjectAttr))
    {
        LOG_ERR << "illegal pHead->nLen = " << static_cast<int32>(pHead->nLen) << std::endl;
        return static_cast<int32>(pHead->nLen);
    }

    auto nAttrSize = pHead->nLen / sizeof(ObjectAttr);
    for (int i = 0; i < nAttrSize; ++i)
    {
        ObjectAttr attr;
        memcpy(&attr, strContent.c_str() + sizeof(DataHeader) + i * sizeof(ObjectAttr), sizeof(ObjectAttr));
        vecAttrs->push_back(attr);
    }

    LOG_INFO << strObjectName << " Attr Size :" << nAttrSize << std::endl;
    return EC_SDK_SUCCESS;
}

/**
 * @brief          get object data
 * @param [in]     objectName  object name
 * @param [out]    vecAttrValues  object attribute  values
 * @return
 * @version        2024/09/25	wuzheqiang	Initial Version
 */
int32 DRSdkManager::DrReadObectData(const std::string &strObjectName, std::vector<drsdk::ObjectData> *vecAttrValues)
{
    auto nSize = strObjectName.size();
    if (0 == nSize)
    {
        LOG_ERR << "strObjectName size is 0." << std::endl;
        return -1;
    }

    std::string strContent = strObjectName;
    DataHeader tHead;
    tHead.nType = SDK_READ_ATTR_DATA;
    tHead.nLen = strContent.size();
    tHead.nSeq = m_nHeadSeq.fetch_add(1);

    int32 nRet = WriteToSendBuffer(tHead, strContent, strContent.size());
    if (0 != nRet)
    {
        LOG_ERR << "WriteToSendBuffer failed. nRet = " << nRet << std::endl;
        return nRet;
    }
    strContent.clear();
    nRet = AsyncGetResult(tHead, &strContent);
    if (0 != nRet)
    {
        LOG_ERR << "AsyncGetResult failed. nRet = " << nRet << std::endl;
        return nRet;
    }
    DataHeader *pHead = reinterpret_cast<DataHeader *>(const_cast<char *>(strContent.c_str()));
    if (pHead->nFlag != 0)
    {
        LOG_ERR << "pHead->nFlag = " << static_cast<int32>(pHead->nFlag) << std::endl;
        return static_cast<int32>(pHead->nFlag);
    }
    std::vector<std::string> vecAttrs;
    SplitString(strContent.substr(sizeof(DataHeader), pHead->nLen), ksDivideMark, &vecAttrs);
    auto nAttrSize = vecAttrs.size();
    LOG_INFO << strObjectName << " Attr Size :" << nAttrSize << std::endl;
    if (nAttrSize % 2 != 0)
    {
        return EC_SDK_COMMON_ERROR;
    }
    for (int i = 0; i < nAttrSize; i += 2)
    {
        vecAttrValues->emplace_back(vecAttrs[i], vecAttrs[i + 1]);
    }
    return EC_SDK_SUCCESS;
}

/**
 * @brief		Write content into  send_buffer
 * @param [in]	tHead   msg head
 * @param [in]	strContent   msg content
 * @param [in]	nLen  msg length
 * @return		0 if success
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
int32 DRSdkManager::WriteToSendBuffer(const DataHeader &tHead, const std::string &strContent, const size_t nLen)
{
    std::lock_guard<std::mutex> lock(m_SendBufferMtx);
    int32 nRet = 0;
    memcpy(m_pSendBuffer.get(), &tHead, sizeof(tHead));
    memcpy(m_pSendBuffer.get() + sizeof(tHead), strContent.c_str(), nLen);

    int32 nLenBuf = sizeof(tHead) + strContent.size();
    nRet = m_pTransport->SendData(m_pSendBuffer.get(), nLenBuf);
    if (0 != nRet)
    {
        LOG_ERR << "send data failed. length = " << nLenBuf << ", nRet = " << nRet << std::endl;
        return nRet;
    }
    LOG_INFO << "send data success. length = " << nLenBuf << ", tHead = " << tHead.ToString() << std::endl;
    SharedPromise call_promise = std::make_shared<Promise>();
    SharedFuture f(call_promise->get_future());
    m_mapRequest[tHead.nSeq] = std::make_tuple(call_promise, f, NowSecond());
    return EC_SDK_SUCCESS;
}

/**
 * @brief          async get response result
 * @param [in]     tHead   msg head
 * @param [out]    pRes    response result
 * @return         0 if success
 * @version        2024/09/23	wuzheqiang	Initial Version
 */
int32 DRSdkManager::AsyncGetResult(const DataHeader &tHead, std::string *pRes)
{
    auto &tuple = m_mapRequest[tHead.nSeq];
    auto &future = std::get<1>(tuple);
    auto status = future.wait_for(std::chrono::milliseconds(m_tDrSdkConfig.nTimeoutMs));

    int nRet = EC_SDK_SUCCESS;
    if (status != std::future_status::ready)
    {
        LOG_ERR << "time_out, tHead: " << tHead.ToString() << std::endl;
        nRet = EC_SDK_TIMEOUT;
    }
    else
    {
        *pRes = future.get();
    }
    {
        std::lock_guard<std::mutex> lg(m_RequestMtx);
        m_mapRequest.erase(tHead.nSeq);
    }

    return nRet;
};

/**
 * @brief		callback function that loops through to get data
 * @version		09/10/2024	wuzheqiang	Initial Version
 */
void DRSdkManager::RecvThreadCallback()
{
    while (!m_bStop)
    {
        int32 buffer_len = RECV_BUFFER_MAX_LENGTH;
        int32 nRet = m_pTransport->RecvData(m_pRecvBuffer.get(), buffer_len);
        if (0 != nRet)
        {
            LOG_ERR << "recv data failed. nRet = " << nRet << std::endl;
        }
        LOG_INFO << "recv data. length = " << buffer_len << std::endl;

        DataHeader *pHead = (DataHeader *)m_pRecvBuffer.get();
        LOG_INFO << "recv data. head: " << pHead->ToString() << std::endl;
        switch (pHead->nType)
        {
        case SDK_CONTROL_DATA:
        case SDK_DUMP_DATA:
        case SDK_READ_DATA:
        case SDK_REG_TAG:
        case SDK_READ_ATTR:
        case SDK_READ_ATTR_DATA: {
            std::lock_guard<std::mutex> lg(m_RequestMtx);
            if (m_mapRequest.find(pHead->nSeq) != m_mapRequest.end())
            {
                auto &tuple = m_mapRequest[pHead->nSeq];
                auto &promise = std::get<0>(tuple);
                promise->set_value(std::string(m_pRecvBuffer.get(), sizeof(DataHeader) + pHead->nLen));
            }
            else
            {
                LOG_ERR << "recved msg time_out ,nSeq" << pHead->nSeq << std::endl;
            }
        }
        break;
        case SDK_REG_TAG_DATA: {
            if (0 != pHead->nFlag)
            {
                LOG_ERR << "SDK_REG_TAG_DATA, nFlag = " << static_cast<int32>(pHead->nFlag) << std::endl;
                break;
            }
            auto nBatchId = pHead->nReserve1;
            if (m_mapRegInfos.find(nBatchId) == m_mapRegInfos.end())
            {
                LOG_ERR << "SDK_REG_TAG_DATA, batch id not found. nBatchId = " << nBatchId << std::endl;
                break;
            }
            std::string strContent = std::string(m_pRecvBuffer.get() + sizeof(DataHeader), pHead->nLen);
            std::vector<std::string> vecContent;
            SplitString(strContent, std::string(ksDivideMark), &vecContent);
            if (vecContent.size() % 2 != 0)
            {
                LOG_ERR << "SDK_REG_TAG_DATA, content size error. size() = " << vecContent.size() << std::endl;
                break;
            }
            std::map<std::string, std::string> mapTags;
            for (int32 i = 0; i < vecContent.size(); i += 2)
            {

                std::string &strTagName = vecContent[i];
                std::string &strValue = vecContent[i + 1];
                mapTags.emplace(strTagName, strValue);
            }

            auto func = m_mapRegInfos[nBatchId].pCallback;
            // add task to thread pool and  async execute
            m_pThreadPool->Enqueue(func, mapTags);

            break;
        }
        default: {
            LOG_ERR << "pHead->nType not support. nType = " << pHead->nType << std::endl;
            break;
        }
        }
    }
}
} // namespace drsdk