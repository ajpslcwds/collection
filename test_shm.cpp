// g++ -o test_shm test_shm.cpp -I/home/wzq/code/dr/include/ -I/home/wzq/code/dr/include/iconv -L/home/wzq/code/dr/executable -L/home/wzq/code/dr/library -Wl,--no-as-needed -ldrhdProcComm -ldrhdOS -liconv -lpthread -lrt

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <string.h>
#include <thread>
#include <vector>
#include "hdProcComm.h"
 
 
#define ICV_SUCCESS 0
#define EC_ICV_PROTO_LEN -1
#define EC_ICV_PROTO_VALUE -2
#define CV_MAX_NUMBER_DATATYPE 30
#define PROTO_DIRVIERAPI_HEAD_LEN 16
#define PROTO_DIRVIERAPI_VERSION_DEFAULT 0
#define PROTO_DIRVIERAPI_TProtoIDVTQ_LEN sizeof(int32_t) + sizeof(int32_t) + sizeof(int16_t) + sizeof(int16_t) + sizeof(int32_t) + sizeof(uint8_t)
 
#define TIMED_BLOCK(name)                                                                                                                            \
    struct __TimedBlock_##name                                                                                                                       \
    {                                                                                                                                                \
        std::chrono::time_point<std::chrono::high_resolution_clock> start;                                                                           \
        const char *block_name;                                                                                                                      \
        __TimedBlock_##name(const char *bn) : start(std::chrono::high_resolution_clock::now()), block_name(bn) {}                                    \
        ~__TimedBlock_##name()                                                                                                                       \
        {                                                                                                                                            \
            auto end = std::chrono::high_resolution_clock::now();                                                                                    \
            std::cout << "[" << block_name << "] elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n"; \
        }                                                                                                                                            \
    } __timed_block_##name(#name)
 
constexpr char SHMNAME[] = "hdProcCommTest";
constexpr int32_t SHMSIZE = 64;                     // MB
constexpr int32_t BLOCKSIZE = 4096;
constexpr int32_t QUEUEID = 1;
constexpr int32_t MAX_TAG_CNT = 100000;
constexpr int32_t TIME_INTERVAL = 10;               // ms
constexpr int32_t RECV_BUF_LEN = 512 * 1024;
 
enum
{
    DRIVERAPI_CMD_INVALID = -1,
    DRIVERAPI_CMD_REGISTER_DRIVER,
    DRIVERAPI_CMD_QUERY_RY_STATUS,
    DRIVERAPI_CMD_SAVE_DATA,
    DRIVERAPI_CMD_CTRL,
    DRIVERAPI_CMD_SET_BLOCKQUALITY,
    DRIVERAPI_CMD_SAVE_DRV_STATUS,
    DRIVERAPI_CMD_SAVE_DEV_STATUS,
    DRIVERAPI_CMD_MAX = DRIVERAPI_CMD_SAVE_DEV_STATUS
};
 
struct TProtoDriverAPIHeader
{
public:
    TProtoDriverAPIHeader()
    {
        m_nCmdType = 0;
        m_nVersion = 0;
        m_nReverse1 = 0;
        m_nReverse2 = 0;
    }
    explicit TProtoDriverAPIHeader(int32_t nCmdType)
    {
        m_nCmdType = nCmdType;
        m_nVersion = 0;
        m_nReverse1 = 0;
        m_nReverse2 = 0;
    }
 
    TProtoDriverAPIHeader(int32_t nCmdType, int32_t nVersion)
    {
        m_nCmdType = nCmdType;
        m_nVersion = nVersion;
        m_nReverse1 = 0;
        m_nReverse2 = 0;
    }
    int32_t m_nCmdType;
    int32_t m_nVersion;
    int32_t m_nReverse1;
    int32_t m_nReverse2;
};
 
struct TProtoIDVTQ
{
    int32_t m_nTagID; // tagids
    int32_t m_nSec;
    int16_t m_nMs;
    int16_t m_nQuality;
    int32_t m_nLenBuf;   // buf长度
    uint8_t m_nDataType; // 类型
    char *m_pBuf;        // 内容
};
 
static void proto_driverapi_save_data_pack(const std::vector<TProtoIDVTQ> &v_msg, char **ppBuf, int32_t *pnLen)
{
    int32_t nRecNum = v_msg.size();
    int32_t nLen = sizeof(nRecNum);
    for (int32_t i = 0; i < nRecNum; i++)
    {
        nLen += PROTO_DIRVIERAPI_TProtoIDVTQ_LEN;
        nLen += v_msg[i].m_nLenBuf;
    }
 
    *pnLen = PROTO_DIRVIERAPI_HEAD_LEN + nLen;
    *ppBuf = new char[*pnLen];
    char *pBufPt = *ppBuf;
 
    TProtoDriverAPIHeader header(DRIVERAPI_CMD_SAVE_DATA);
    memcpy(pBufPt, &header, sizeof(TProtoDriverAPIHeader));
    pBufPt += sizeof(TProtoDriverAPIHeader);
 
    memcpy(pBufPt, &nRecNum, sizeof(nRecNum));
    pBufPt += sizeof(nRecNum);
 
    for (int32_t i = 0; i < nRecNum; i++)
    {
        memcpy(pBufPt, &v_msg[i].m_nTagID, sizeof(v_msg[i].m_nTagID));
        pBufPt += sizeof(v_msg[i].m_nTagID);
        memcpy(pBufPt, &v_msg[i].m_nSec, sizeof(v_msg[i].m_nSec));
        pBufPt += sizeof(v_msg[i].m_nSec);
        memcpy(pBufPt, &v_msg[i].m_nMs, sizeof(v_msg[i].m_nMs));
        pBufPt += sizeof(v_msg[i].m_nMs);
        memcpy(pBufPt, &v_msg[i].m_nQuality, sizeof(v_msg[i].m_nQuality));
        pBufPt += sizeof(v_msg[i].m_nQuality);
        memcpy(pBufPt, &v_msg[i].m_nLenBuf, sizeof(v_msg[i].m_nLenBuf));
        pBufPt += sizeof(v_msg[i].m_nLenBuf);
        memcpy(pBufPt, &v_msg[i].m_nDataType, sizeof(v_msg[i].m_nDataType));
        pBufPt += sizeof(v_msg[i].m_nDataType);
 
        memcpy(pBufPt, v_msg[i].m_pBuf, v_msg[i].m_nLenBuf);
        pBufPt += v_msg[i].m_nLenBuf;
    }
}
 
static int32_t proto_driverapi_save_data_unpack(char *pBuf, int32_t nLen, std::vector<TProtoIDVTQ> *v_msg)
{
    pBuf += PROTO_DIRVIERAPI_HEAD_LEN;
    nLen -= PROTO_DIRVIERAPI_HEAD_LEN;
    if (nLen < 0)
    {
        return EC_ICV_PROTO_LEN;
    }
 
    int32_t nRecNum = 0;
 
    nLen -= sizeof(nRecNum);
    if (nLen < 0)
    {
        return EC_ICV_PROTO_LEN;
    }
    memcpy(&nRecNum, pBuf, sizeof(nRecNum));
    if (nRecNum <= 0)
    {
        return EC_ICV_PROTO_VALUE;
    }
    pBuf += sizeof(nRecNum);
 
    v_msg->resize(nRecNum);
 
    for (int32_t i = 0; i < nRecNum; i++)
    {
        nLen -= PROTO_DIRVIERAPI_TProtoIDVTQ_LEN;
        if (nLen < 0)
        {
            return EC_ICV_PROTO_LEN;
        }
        TProtoIDVTQ rec;
        memcpy(&rec.m_nTagID, pBuf, sizeof(rec.m_nTagID));
        pBuf += sizeof(rec.m_nTagID);
        memcpy(&rec.m_nSec, pBuf, sizeof(rec.m_nSec));
        pBuf += sizeof(rec.m_nSec);
        memcpy(&rec.m_nMs, pBuf, sizeof(rec.m_nMs));
        pBuf += sizeof(rec.m_nMs);
        memcpy(&rec.m_nQuality, pBuf, sizeof(rec.m_nQuality));
        pBuf += sizeof(rec.m_nQuality);
        memcpy(&rec.m_nLenBuf, pBuf, sizeof(rec.m_nLenBuf));
        pBuf += sizeof(rec.m_nLenBuf);
        memcpy(&rec.m_nDataType, pBuf, sizeof(rec.m_nDataType));
        pBuf += sizeof(rec.m_nDataType);
 
        if (rec.m_nDataType >= CV_MAX_NUMBER_DATATYPE)
        {
            return EC_ICV_PROTO_VALUE;
        }
 
        if (rec.m_nLenBuf < 0)
        {
            return EC_ICV_PROTO_VALUE;
        }
        nLen -= rec.m_nLenBuf;
        if (nLen < 0)
        {
            return EC_ICV_PROTO_LEN;
        }
        rec.m_pBuf = pBuf;
        pBuf += rec.m_nLenBuf;
 
        (*v_msg)[i] = rec;
    }
 
    return ICV_SUCCESS;
}
 
std::vector<std::vector<TProtoIDVTQ>> g_tagArr;
char *g_buf = nullptr;
std::atomic<bool> g_bStopFlag(false);
 
void InitializeVec()
{
    int nSize = sizeof(int16_t) * 50000 + sizeof(float) * 50000;
    if (nullptr != g_buf)
        delete[] g_buf;
 
    g_buf = new char[nSize];
    memset(g_buf, 0, nSize);
 
    g_tagArr.resize(10);
 
    for (int i = 0; i < 10; i++)
    {
        g_tagArr[i].resize(MAX_TAG_CNT / 10);
 
        for (int j = 0; j < MAX_TAG_CNT / 10; j++)
        {
            g_tagArr[i][j].m_nTagID = i;
            g_tagArr[i][j].m_nQuality = 0;
 
            char *pBuf = g_buf;
 
            if (i < 5)
            {
                g_tagArr[i][j].m_nDataType = 1; // INT
                g_tagArr[i][j].m_nLenBuf = sizeof(int16_t);
                g_tagArr[i][j].m_pBuf = pBuf;
                pBuf += sizeof(int16_t);
            }
            else
            {
                g_tagArr[i][j].m_nDataType = 3; // FLOAT
                g_tagArr[i][j].m_nLenBuf = sizeof(float);
                g_tagArr[i][j].m_pBuf = pBuf;
                pBuf += sizeof(float);
            }
        }
    }
}
 
void signalHandler(int signal)
{
    if (signal == SIGINT)
    {
        std::cout << "\nCaught Ctrl+C (SIGINT). Exiting loop..." << std::endl;
        g_bStopFlag.store(true);
    }
}
 
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <recv/send>" << std::endl;
        return 1;
    }
 
    std::signal(SIGINT, signalHandler);
 
    char *buf = new char[512 * 1024]{};
    int nBufLen = RECV_BUF_LEN;
 
    if (strcmp(argv[1], "send") == 0)
    {
        InitializeVec();
 
        ProcCommHandle hProcComm;
        ProcCommQueueHandle hQueue;
        int32_t nRet = proccomm_init(SHMNAME, SHMSIZE, BLOCKSIZE, 0, &hProcComm);
        std::cout << "proccomm_init: " << nRet << std::endl;
        nRet = proccomm_reg(hProcComm, QUEUEID, &hQueue);
        std::cout << "proccomm_reg: " << nRet << std::endl;
 
        int16_t nValInt = 100;
        float nValFloat = 100.0f;
 
        while (!g_bStopFlag.load())
        {
            std::chrono::_V2::system_clock::time_point now = std::chrono::system_clock::now();
            int64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
            int32_t nSec = static_cast<int32_t>(ms / 1000);
            int16_t nMs = static_cast<int16_t>(ms % 1000);
 
            {
                TIMED_BLOCK(send);
 
                char *pBuf = nullptr;
 
                nValInt = -nValInt;
                nValFloat = -nValFloat;
 
                for (int i = 0; i < 10; i++)
                {
                    for (int j = 0; j < MAX_TAG_CNT / 10; j++)
                    {
                        g_tagArr[i][j].m_nSec = nSec;
                        g_tagArr[i][j].m_nMs = nMs;
 
                        if (i < 5)
                            memcpy(g_tagArr[i][j].m_pBuf, &nValInt, sizeof(int16_t));
                        else
                            memcpy(g_tagArr[i][j].m_pBuf, &nValFloat, sizeof(float));
                    }
 
                    proto_driverapi_save_data_pack(g_tagArr[i], &pBuf, &nBufLen);
                    nRet = proccomm_send(hQueue, pBuf, nBufLen);
                    std::cout << "proccomm_send: " << nRet << ", BufLen: " << nBufLen << std::endl;
 
                    delete[] pBuf;
                    pBuf = nullptr;
                }
            }
 
            std::this_thread::sleep_until(now + std::chrono::milliseconds(TIME_INTERVAL));
        }
 
        nRet = proccomm_release(hProcComm);
        std::cout << "proccomm_release: " << nRet << std::endl;
    }
    else if (strcmp(argv[1], "recv") == 0)
    {
        ProcCommHandle hProcComm;
        ProcCommQueueHandle hQueue;
        int32_t nRet = proccomm_init(SHMNAME, SHMSIZE, BLOCKSIZE, 0, &hProcComm);
        std::cout << "proccomm_init: " << nRet << std::endl;
        nRet = proccomm_reg(hProcComm, QUEUEID, &hQueue);
        std::cout << "proccomm_reg: " << nRet << std::endl;
 
        while (!g_bStopFlag.load())
        {
            TIMED_BLOCK(recv);
 
            nBufLen = RECV_BUF_LEN;
            nRet = proccomm_recv(hQueue, buf, &nBufLen, true, 100);
            std::cout << "recv: " << nRet << ", Len:" << nBufLen << std::endl;
 
            if (nRet != 0)
                continue;
 
            std::vector<TProtoIDVTQ> v_msg;
            nRet = proto_driverapi_save_data_unpack(buf, nBufLen, &v_msg);
            std::cout << "proto_driverapi_save_data_unpack: " << nRet << ", msg size: " << v_msg.size() << std::endl;
        }
 
        nRet = proccomm_recv_quit(hQueue);
        std::cout << "proccomm_recv_quit: " << nRet << std::endl;
        nRet = proccomm_release(hProcComm);
        std::cout << "proccomm_release: " << nRet << std::endl;
    }
 
    if (nullptr != g_buf)
    {
        delete[] g_buf;
        g_buf = nullptr;
    }
 
    if (nullptr != buf)
    {
        delete[] buf;
        buf = nullptr;
    }
 
    return 0;
}