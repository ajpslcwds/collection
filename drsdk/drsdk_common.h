/*  Filename:    drsdk_common.h
 *  Copyright:   Shanghai Baosight Software Co., Ltd.
 *
 *  Description: Define  common  structure, function and macro
 *
 *  @author:     wuzheqiang
 *	@version     09/10/2024	wuzheqiang	Initial Version
 **************************************************************/
#ifndef DRSDK_COMMON_H
#define DRSDK_COMMON_H

#include "data_types.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>

namespace drsdk
{
constexpr uint32 BUFFER_MAX_LENGTH = 1024;
constexpr uint32 SEND_BUFFER_MAX_LENGTH = BUFFER_MAX_LENGTH * 64;
constexpr uint32 RECV_BUFFER_MAX_LENGTH = BUFFER_MAX_LENGTH * 64;

/**
 * @brief          split string,
 *                  e.g. "name1@@value1@@name2@@value2@@" ->  {"name1","value1","name2","value2"}
 *                  e.g. "name1@@value1@@name2@@value2"   ->  {"name1","value1","name2","value2"}
 * @param [in]     strContent      string to be split
 * @param [in]     strDivideMarkz  divide mark
 * @param [out]    vecRes          result
 * @version        2024/09/23	wuzheqiang	Initial Version
 */
inline void SplitString(const std::string &strContent, const std::string &strDivideMark,
                        std::vector<std::string> *vecRes)
{
    size_t nStart = 0;
    size_t nEnd = 0;
    vecRes->clear();
    while (nStart < strContent.size() && nEnd != std::string::npos) // make sure not overflow
    {
        nEnd = strContent.find(strDivideMark, nStart);
        if (nEnd != nStart)
        {
            vecRes->push_back(strContent.substr(nStart, nEnd - nStart));
        }
        nStart = nEnd + strDivideMark.size();
    }
}

/*get cur second time_stamp*/
inline uint32 NowSecond()
{
    using namespace std::chrono;
    auto now = system_clock::now();

    auto seconds_since_epoch = duration_cast<seconds>(now.time_since_epoch()).count();

    return static_cast<uint32>(seconds_since_epoch);
}

/*get cur microseconds time_stamp str*/
inline std::string CurrentTimestampMicro()
{
    using namespace std::chrono;
    auto now = system_clock::now();

    auto in_time_t = system_clock::to_time_t(now);
    auto us = duration_cast<microseconds>(now.time_since_epoch()) % 1000000;
    std::tm buf;
    localtime_r(&in_time_t, &buf);
    std::ostringstream oss;
    oss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(6) << us.count();
    return oss.str();
}

#define RESET "\033[0m"
#define RED "\033[31m"    /* Red */
#define YELLOW "\033[33m" /* Yellow */
#define FILENAME (std::string(__FILE__).substr(std::string(__FILE__).find_last_of("/\\") + 1))
#define LOG_INFO                                                                                                       \
    std::cout << RESET << "[" << drsdk::CurrentTimestampMicro() << "]DRSDK_INFO[" << FILENAME << ":" << __LINE__ << "]"
#define LOG_ERR                                                                                                        \
    std::cerr << RED << "[" << drsdk::CurrentTimestampMicro() << "]DRSDK_ERR[" << FILENAME << ":" << __LINE__ << "]"

enum SDKErrorCode
{
    EC_SDK_SUCCESS = 0,
    EC_SDK_BASE = 1000,                                // the base of sdk error code
    EC_SDK_COMMON_ERROR = (EC_SDK_BASE + 1),           // common error
    EC_SDK_CONNECT_FAILED = (EC_SDK_BASE + 2),         // connect failed
    EC_SDK_MALLOC_FAILED = (EC_SDK_BASE + 3),          // malloc failed
    EC_SDK_TIMEOUT = (EC_SDK_BASE + 4),                //  timeout. socket or other
    EC_SDK_INCORRECT_INPUT_PARAMS = (EC_SDK_BASE + 5), // incorrect input params
};

enum SDKReqType : uint8
{
    SDK_CONTROL_DATA = 1,
    SDK_DUMP_DATA = 2,
    SDK_REG_TAG = 3,
    SDK_REG_TAG_DATA = 4,
    SDK_READ_DATA = 5,
    SDK_READ_ATTR = 6,
    SDK_READ_ATTR_DATA = 7,
};

enum SDKAttrType : uint8
{
    SDK_ATTR_TYPE_unknown = 0,
    SDK_ATTR_TYPE_INT8,
    SDK_ATTR_TYPE_INT16,
    SDK_ATTR_TYPE_INT32,
    SDK_ATTR_TYPE_INT64,
    SDK_ATTR_TYPE_UINT8,
    SDK_ATTR_TYPE_UINT16,
    SDK_ATTR_TYPE_UINT32,
    SDK_ATTR_TYPE_UINT64,
    SDK_ATTR_TYPE_FLOAT,
    SDK_ATTR_TYPE_DOUBLE,
    SDK_ATTR_TYPE_STRING,
    SDK_ATTR_TYPE_BOOL,
    SDK_ATTR_TYPE_STRUCT,
    SDK_ATTR_TYPE_max,
};

#pragma pack(push, 4) // set pack mode
struct DataHeader
{
    uint8 nType = 0;      // request/response type. use enum SDKReqType
    uint8 nFlag = 0;      // result flag. 0 success, 1 fail
    uint16 nLen = 0;      // content data len
    uint32 nSeq = 0;      // for async request
    uint16 nReserve1 = 0; // reserve. special purpose
    uint16 nReserve2 = 0; // reserve. special purpose

    std::string ToString() const
    {
        std::stringstream ss;
        ss << "head_size:" << sizeof(DataHeader) << ", nType:" << (int32)nType << ", nFlag:" << (int32)nFlag
           << ", nLen:" << (int32)nLen << ", nSeq:" << nSeq << ", nReserve1:" << (int32)nReserve1
           << ", nReserve2:" << (int32)nReserve2;
        return ss.str();
    }
};
#pragma pack(pop) // resume to default
struct ObjectAttr
{
    char sName[256] = {0};      // attr name
    char sAliasName[256] = {0}; // attr alias name
    SDKAttrType nType = SDK_ATTR_TYPE_unknown;
    uint16 nLen = 0; // attr data len

    std::string ToString() const
    {
        std::stringstream ss;
        ss << "name:" << sName << ", alias:" << sAliasName << ", type:" << (int32)nType << ", len:" << nLen;
        return ss.str();
    }
};

struct ObjectData
{
    std::string sName = "";  // attr name
    std::string sValue = ""; // attr value

    ObjectData(const std::string &name, const std::string &value) : sName(name), sValue(value)
    {
    }

    std::string ToString() const
    {
        std::stringstream ss;
        ss << "name:" << sName << ", sValue:" << sValue;
        return ss.str();
    }
};

struct DRSdkConfig
{
    std::string strIp = "127.0.0.1";
    int16 nPort = 1224;
    uint16 nTimeoutMs = 1000; // ms
    uint32 nThreadPoolNum = 10;

    std::string ToString() const
    {
        std::stringstream ss;
        ss << "ip:" << strIp << ", port:" << nPort << ", timeout:" << nTimeoutMs << ", threadpool:" << nThreadPoolNum;
        return ss.str();
    }
};

} // namespace drsdk
#endif