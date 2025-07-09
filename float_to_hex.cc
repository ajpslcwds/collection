#include "json.hpp"
#include <atomic>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <stdio.h>
#include <string.h>
#include <string>
#include <thread>
#include <vector>

using namespace std;

inline std::string FloatToAutoString(float value, int precision = 7)
{
    std::ostringstream oss;
    oss << std::defaultfloat << std::setprecision(precision);
    oss << value;
    return oss.str();
}

inline std::string DoubleToAutoString(double value, int precision = 17)
{
    std::ostringstream oss;
    oss << std::defaultfloat << std::setprecision(precision);
    oss << value;
    return oss.str();
}

int test_to_string()
{
    {
        double d = -1.7976931348623157e+308;
        std::cout << d << std::endl;
        std::cout << std::to_string(d) << std::endl;
        printf("%f\n", d);
        printf("%e\n", d);
        printf("%g\n", d);
        printf("%.7g\n", d);
        std::cout << "---------------------------------" << std::endl;
    }

    {
        std::vector<float> vec = {0.0, 123.456, 0.000000123, 3.402823e+38, 1.175495e-38, -3.402823e+38, -1.175495e-38};
        for (auto &f : vec)
            std::cout << "f = " << FloatToAutoString(f) << std::endl;
        std::cout << "---------------------------------" << std::endl;
    }

    {
        std::vector<double> vec = {0.0,
                                   123.456,
                                   0.000000123,
                                   3.402823e+38,
                                   1.175495e-38,
                                   2.2250738585072014e-308,
                                   1.7976931348623157e+308,
                                   -2.2250738585072014e-308,
                                   -1.7976931348623157e+308};
        for (auto &d : vec)
            std::cout << "d = " << DoubleToAutoString(d) << std::endl;

        std::cout << "---------------------------------" << std::endl;
    }

    {
        double x = 1.0 / 0.0;
        std::cout << "std::cout: " << x << std::endl;
        std::string s = std::to_string(x);
        std::cout << "std::to_string: " << s << std::endl;
        std::cout << "---------------------------------" << std::endl;
    }

    return 0;
}
class FloatHexConverter
{
  public:
    // 将float转换为16进制字符串
    static std::string FloatToHex(float value)
    {
        uint32_t bits;
        std::memcpy(&bits, &value, sizeof(float));
        std::stringstream ss;
        ss << std::hex << std::setw(8) << std::setfill('0') << bits;
        return ss.str();
    }

    // 将double转换为16进制字符串
    static std::string DoubleToHex(double value)
    {
        uint64_t bits;
        std::memcpy(&bits, &value, sizeof(double));
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << bits;
        return ss.str();
    }

    // 将16进制字符串转换为float
    static float HexToFloat(const std::string &hexStr)
    {
        try
        {
            // 移除可能的"0x"前缀
            std::string cleanHex = hexStr;
            if (hexStr.substr(0, 2) == "0x" || hexStr.substr(0, 2) == "0X")
            {
                cleanHex = hexStr.substr(2);
            }

            // 检查输入长度
            if (cleanHex.length() > 8)
            {
                throw std::invalid_argument("Hex string too long for float:" + hexStr);
            }

            // 转换为32位无符号整数
            uint32_t bits;
            std::stringstream ss(cleanHex);
            ss >> std::hex >> bits;

            if (ss.fail())
            {
                throw std::invalid_argument("Invalid hex string:" + hexStr);
            }

            // 转换回float
            float result;
            std::memcpy(&result, &bits, sizeof(float));
            return result;
        }
        catch (const std::exception &e)
        {
            throw std::invalid_argument("Failed to convert hex to float: " + std::string(e.what()));
        }
    }

    // 将16进制字符串转换为double
    static double HexToDouble(const std::string &hexStr)
    {
        try
        {
            // 移除可能的"0x"前缀
            std::string cleanHex = hexStr;
            if (hexStr.substr(0, 2) == "0x" || hexStr.substr(0, 2) == "0X")
            {
                cleanHex = hexStr.substr(2);
            }

            // 检查输入长度
            if (cleanHex.length() > 16)
            {
                throw std::invalid_argument("Hex string too long for double:" + hexStr);
            }

            // 转换为64位无符号整数
            uint64_t bits;
            std::stringstream ss(cleanHex);
            ss >> std::hex >> bits;

            if (ss.fail())
            {
                throw std::invalid_argument("Invalid hex string:" + hexStr);
            }

            // 转换回double
            double result;
            std::memcpy(&result, &bits, sizeof(double));
            return result;
        }
        catch (const std::exception &e)
        {
            throw std::invalid_argument("Failed to convert hex to double: " + std::string(e.what()));
        }
    }
};

int main()
{

    std::vector<float> fs = {1.175495e-38, 3.402823e+38, -1.175495e-38, -3.402823e+38};
    for (float f : fs)
    {
        std::cout << "Float: " << f << std::endl;
        std::cout << "Hex: " << FloatHexConverter::FloatToHex(f) << std::endl;
    }
    std::cout << "---------------------------------" << std::endl;

    std::vector<double> ds = {2.225073858507201e-308, 1.7976931348623157e+308, -2.225073858507201e-308,
                              -1.7976931348623157e+308};
    for (double d : ds)
    {
        std::cout << "Double: " << DoubleToAutoString(d) << std::endl;
        std::cout << "Hex: " << FloatHexConverter::DoubleToHex(d) << std::endl;
    }

    std::cout << "---------------------------------" << std::endl;

    std::vector<std::string> dss = {"000fffffffffffff", "7fefffffffffffff", "800fffffffffffff", "ffefffffffffffff",
                                    "3ff0000000000000"};
    for (std::string &ds : dss)
    {
        std::cout << "Hex: " << ds << std::endl;
        std::cout << "Double: " << DoubleToAutoString(FloatHexConverter::HexToDouble(ds)) << std::endl;
    }

    return 0;
}