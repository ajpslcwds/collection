#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <type_traits>

template <typename T> struct always_false : std::false_type
{
};

template <typename T> void WriteToBuffer(const std::string &valueStr, char *buffer, int &pos)
{
    T value;
    if constexpr (std::is_same_v<T, int32_t>)
    {
        value = static_cast<int32_t>(std::atoi(valueStr.c_str()));
    }
    else if constexpr (std::is_same_v<T, uint32_t>)
    {
        value = static_cast<uint32_t>(std::stoul(valueStr));
    }
    else if constexpr (std::is_same_v<T, float>)
    {
        value = static_cast<float>(std::atof(valueStr.c_str()));
    }
    else if constexpr (std::is_same_v<T, std::string>)
    {
        value = valueStr;
    }
    else
    {
        static_assert(always_false<T>::value, "Unsupported type");
    }

    if constexpr (std::is_same_v<T, std::string>)
    {
        std::memcpy(buffer + pos, value.c_str(), value.size());
        pos += value.size();
    }
    else
    {
        std::memcpy(buffer + pos, &value, sizeof(value));
        pos += sizeof(value);
    }
}

int main()
{
    char buffer[1024] = {0};
    int pos = 0;

    // Input values
    WriteToBuffer<int32_t>("-123", buffer, pos);
    WriteToBuffer<uint32_t>("221", buffer, pos);
    WriteToBuffer<float>("12.12", buffer, pos);
    WriteToBuffer<std::string>("abcde", buffer, pos);

    // Print buffer content for verification (as hex)
    for (int i = 0; i < pos; ++i)
    {
        printf("%02x ", static_cast<unsigned char>(buffer[i]));
    }
    printf("\n");

    return 0;
}
