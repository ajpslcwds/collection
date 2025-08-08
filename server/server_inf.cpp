#include "../dsf_ice_inf.h"
#include <Ice/Ice.h>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;
using namespace std;
using namespace DSF;
inline uint64_t NowMillisecond()
{
    using namespace std::chrono;
    auto now = system_clock::now();

    auto milliseconds_since_epoch = duration_cast<milliseconds>(now.time_since_epoch()).count();

    return static_cast<uint64_t>(milliseconds_since_epoch);
}

class DsfIceInfI : public DSF::DsfIceInf
{
  public:
    virtual bool queryRmStatus(const Ice::Current &current) override;

    virtual DSF::ReadResult drRead(const std::vector<std::string> &names, const Ice::Current &current) override;

    virtual int drSave(const std::vector<DSF::TagValue> &values, const Ice::Current &current) override;
};

bool DsfIceInfI::queryRmStatus(const Ice::Current &current)
{
    std::cout << "[queryRmStatus] Called." << std::endl;

    auto endpoint = current.con->getInfo();
    auto tcpInfo = Ice::TCPConnectionInfoPtr::dynamicCast(endpoint);

    if (tcpInfo)
    {
        std::string clientIP = tcpInfo->remoteAddress;
        int clientPort = tcpInfo->remotePort;
        printf("Client IP: %s, Port: %d\n", clientIP.c_str(), clientPort);
    }
    return true; // 假设系统总是处于 active 状态
}

DSF::ReadResult DsfIceInfI::drRead(const std::vector<std::string> &names, const Ice::Current &current)
{
    std::cout << "[drRead] Reading " << names.size() << " tags." << std::endl;

    auto endpoint = current.con->getInfo();
    auto tcpInfo = Ice::TCPConnectionInfoPtr::dynamicCast(endpoint);

    if (tcpInfo)
    {
        std::string clientIP = tcpInfo->remoteAddress;
        int clientPort = tcpInfo->remotePort;
        printf("Client IP: %s, Port: %d\n", clientIP.c_str(), clientPort);
    }

    DSF::ReadResult result;
    result.errCode = 0;

    for (const auto &name : names)
    {
        DSF::TagValue tag;
        tag.name = name;
        tag.type = "DINT"; // 模拟一个类型
        tag.errCode = 0;

        // 假设我们给出一个固定的值，例如 DINT 类型的值 1234
        int val = 1234;
        Ice::ByteSeq byteVal(reinterpret_cast<Ice::Byte *>(&val), reinterpret_cast<Ice::Byte *>(&val) + sizeof(int));

        tag.value = byteVal;
        result.values.push_back(tag);
    }

    return result;
}

int DsfIceInfI::drSave(const std::vector<DSF::TagValue> &values, const Ice::Current &current)
{
    std::cout << "[drSave] Saving " << values.size() << " tags." << std::endl;
    
    auto endpoint = current.con->getInfo();
    auto tcpInfo = Ice::TCPConnectionInfoPtr::dynamicCast(endpoint);

    if (tcpInfo)
    {
        std::string clientIP = tcpInfo->remoteAddress;
        int clientPort = tcpInfo->remotePort;
        printf("Client IP: %s, Port: %d\n", clientIP.c_str(), clientPort);
    }

    for (const auto &tag : values)
    {
        std::cout << "Tag Name: " << tag.name << ", Type: " << tag.type << ", Value Size: " << tag.value.size()
                  << ", errCode: " << tag.errCode << std::endl;
    }

    return 0; // success
}

class Server
{
  public:
    int Run()
    {
        try
        {
            int argc = 0;
            const char **argv = nullptr;
            Ice::CommunicatorHolder ich(argc, argv);
            //   "default -h 127.0.0.1 -p 61245"
            auto adapter = ich->createObjectAdapterWithEndpoints("DsfIceInfAdapter", "default  -p 61245");
            Ice::ObjectPtr receiver = new DsfIceInfI;
            adapter->add(receiver, Ice::stringToIdentity("DSF/DSFIceInf"));
            adapter->activate();

            ich->waitForShutdown();
            return 0;
        }
        catch (const Ice::Exception &ex)
        {
            cerr << ex << endl;
            return 1;
        }
    }
};

int main(int argc, char *argv[])
{
    Server server;
    server.Run();

    return 0;
}
