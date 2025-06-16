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

std::string DataToHexString(const std::string &data)
{
    const char hexDigits[] = "0123456789ABCDEF";
    std::string result;
    result.reserve(data.size() * 2);

    for (unsigned char c : data)
    {
        result += hexDigits[c >> 4];   // high
        result += hexDigits[c & 0x0F]; // low
    }

    return result;
}

std::string HexStringToData(const std::string &hex)
{
    try
    {
        if (hex.size() % 2 != 0)
        {
            throw std::invalid_argument(std::to_string(hex.size()) + ":Hex string must have even length");
        }

        std::string result;
        result.reserve(hex.size() / 2);
        auto hexCharToValue = [](char ch) -> int {
            if ('0' <= ch && ch <= '9')
                return ch - '0';
            if ('A' <= ch && ch <= 'F')
                return ch - 'A' + 10;
            if ('a' <= ch && ch <= 'f')
                return ch - 'a' + 10;
            throw std::invalid_argument(std::string(1, ch) + ":Invalid hex character");
        };

        for (std::size_t i = 0; i < hex.size(); i += 2)
        {
            int high = hexCharToValue(hex[i]);
            int low = hexCharToValue(hex[i + 1]);
            result += static_cast<char>((high << 4) | low);
        }
        return result;
    }
    catch (const std::exception &e)
    {
        std::cerr << "hex:" << hex << ", error_msg:" << e.what() << std::endl;
        return "";
    }
}

void test()
{
    {
        const char raw1[] = {'\x61', '\x62', '\x00', '\x7f', '\xff'};
        std::string raw(raw1, sizeof(raw1));
        std::string hexStr = DataToHexString(raw);
        std::cout << hexStr << std::endl;
    }

    {
        std::string raw = "大宝信baosight123";
        std::string hexStr = DataToHexString(raw);
        std::cout << hexStr << std::endl;
    }
}

void test2()
{
    std::string hexStr = "x5A4A7E5AE9DE4BFA162616F7369676874313233";
    std::string str = HexStringToData(hexStr);
    std::cout << str << std::endl;
}

int main()
{
    test();
    test2();

    return 0;
}