#include "../dsf_ice_inf.h"
#include <Ice/Ice.h>
#include <chrono>
#include <thread>

#define ICE_DRV_CATCH                                                                                                  \
    catch (const std::exception &ex)                                                                                   \
    {                                                                                                                  \
        printf("std exception: %s", ex.what());                                                                        \
        return 1;                                                                                                      \
    }                                                                                                                  \
    catch (...)                                                                                                        \
    {                                                                                                                  \
        printf("unknown exception");                                                                                   \
        return 1;                                                                                                      \
    }

inline std::string StrToHexString(const ::Ice::ByteSeq &data)
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
class IceClient
{
  public:
    IceClient()
    {
    }
    ~IceClient()
    {
        DisConnect();
    }
    int Connect();
    int DisConnect();
    int Test(uint32_t t);

  private:
    Ice::CommunicatorHolder _communicator;
    DSF::DsfIceInfPrx _dsfapiinf;
};

int IceClient::Connect()
{
    try
    {
        int argc = 0;
        char **argv = nullptr;
        Ice::InitializationData initData;
        initData.properties = Ice::createProperties();
        initData.properties->setProperty("Ice.Default.EncodingVersion", "1.0");
        _communicator = Ice::initialize(argc, argv, initData);
        auto base = _communicator->stringToProxy("DSF/DSFIceInf:default -h 192.168.30.218 -p 55011");
        _dsfapiinf = DSF::DsfIceInfPrx::checkedCast(base);
        if (!_dsfapiinf)
        {
            throw std::runtime_error("Invalid proxy");
        }
        return 0;
    }
    ICE_DRV_CATCH
}

int IceClient::DisConnect()
{
    try
    {
        _communicator->destroy();
        return 0;
    }
    ICE_DRV_CATCH
}

int IceClient::Test(uint32_t t)
{
    try
    {
        // 调用 queryRmStatus
        bool status = _dsfapiinf->queryRmStatus();
        std::cout << "[Client] queryRmStatus result: " << (status ? "active" : "inactive") << std::endl;

        // 调用 drRead
        Ice::StringSeq names = {"STD::DS_TLINT", "STD::DS_TLREAL", "STD::DS_TSTR", "STD::DS_TBOOL"};
        DSF::ReadResult readResult = _dsfapiinf->drRead(names);

        std::cout << "[Client] drRead errCode: " << readResult.errCode << std::endl;
        for (const auto &tag : readResult.values)
        {
            std::cout << "  Tag: " << tag.name << ", type: " << tag.type << ", errCode: " << tag.errCode
                      << ", value (hex): " << StrToHexString(tag.value) << std::endl;

            if (0 == tag.errCode)
            {
                if (tag.name.find("LINT") != std::string::npos)
                {
                    long long value = 0;
                    std::memcpy(&value, tag.value.data(), sizeof(value));
                    std::cout << "  Tag: " << tag.name << ", type: " << tag.type << ", errCode: " << tag.errCode
                              << ", value: " << value << std::endl;
                }
                else if (tag.name.find("LREAL") != std::string::npos)
                {
                    double value = 0;
                    std::memcpy(&value, tag.value.data(), sizeof(value));
                    std::cout << "  Tag: " << tag.name << ", type: " << tag.type << ", errCode: " << tag.errCode
                              << ", value: " << value << std::endl;
                }
                else if (tag.name.find("BOOL") != std::string::npos)
                {
                    bool value = 0;
                    std::memcpy(&value, tag.value.data(), sizeof(value));
                    std::cout << "  Tag: " << tag.name << ", type: " << tag.type << ", errCode: " << tag.errCode
                              << ", value: " << value << std::endl;
                }
                else if (tag.name.find("TSTR") != std::string::npos)
                {
                    //  string 有两个字节的头
                    std::cout << "  Tag: " << tag.name << ", type: " << tag.type << ", errCode: " << tag.errCode
                    std::string res((const char*)tag.value.data() + 2, tag.value.data()[1]);
                    printf("%d,%d,%s \n", tag.value.data()[0], tag.value.data()[1], res.c_str());
                }
            }
        }

        // 准备 drWrite 数据
        DSF::TagValueSeq values;
        {
            DSF::TagValue tag;
            tag.name = "STD::DS_TLINT";
            tag.type = "";
            tag.errCode = 0;
            long long value = t;
            tag.value.resize(sizeof(value));
            std::memcpy(tag.value.data(), &value, sizeof(value));
            values.push_back(tag);
        }
        {
            DSF::TagValue tag;
            tag.name = "STD::DS_TLREAL";
            tag.type = "";
            tag.errCode = 0;
            double value = t + 1.01;
            tag.value.resize(sizeof(value));
            std::memcpy(tag.value.data(), &value, sizeof(value));
            values.push_back(tag);
        }
        {
            DSF::TagValue tag;
            tag.name = "STD::DS_TBOOL";
            tag.type = "";
            tag.errCode = 0;
            bool value = t % 2;
            tag.value.resize(sizeof(value));
            std::memcpy(tag.value.data(), &value, sizeof(value));
            values.push_back(tag);
        }
        {
            DSF::TagValue tag;
            tag.name = "STD::DS_TSTR";
            tag.type = "";
            tag.errCode = 0;
            char value[32] = {};
            std::string str = "xiong_" + std::to_string(t);
            value[0] = 30;
            value[1] = str.size();
            memcpy(value + 2, str.c_str(), str.size());
            tag.value.resize(sizeof(value));
            std::memcpy(tag.value.data(), &value, sizeof(value));
            values.push_back(tag);
        }

        int writeResult = _dsfapiinf->drSave(values);
        std::cout << "[Client] drSave result: " << writeResult << std::endl;
        return 0;
    }
    ICE_DRV_CATCH
}

int main(int argc, char *argv[])
{
    try
    {
        IceClient client;
        client.Connect();

        uint32_t t = 0;
        while (true)
        {
            client.Test(++t);
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}