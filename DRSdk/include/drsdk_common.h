#ifndef DRSDK_COMMON_H
#define DRSDK_COMMON_H

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>

namespace drsdk
{
inline uint32_t NowSecond()
{
    using namespace std::chrono;
    auto now = system_clock::now();

    auto seconds_since_epoch = duration_cast<seconds>(now.time_since_epoch()).count();

    return static_cast<uint32_t>(seconds_since_epoch);
}
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

#define FILENAME (std::string(__FILE__).substr(std::string(__FILE__).find_last_of("/\\") + 1))

#define LOG_INFO std::cout << "[" << drsdk::CurrentTimestampMicro() << "][" << FILENAME << ":" << __LINE__ << "]"
#define LOG_ERR std::cerr << "[" << drsdk::CurrentTimestampMicro() << "][" << FILENAME << ":" << __LINE__ << "]"

enum SDKReqType : uint8_t
{
    SDK_CONTROL_DATA = 1,
    SDK_READ_DATA = 2,
    SDK_GET_ID = 3,
    SDK_REG_TAG = 4,
};
struct DataHeader
{
    uint8_t type = 0;
    uint8_t flag = 0;
    uint16_t len = 0;
    uint32_t seq = 0;

    void output()
    {
        LOG_INFO << "type:" << (int)type << ", flag:" << (int)flag << ", len:" << (int)len << ", seq:" << seq
                 << std::endl;
    }
};
} // namespace drsdk
#endif