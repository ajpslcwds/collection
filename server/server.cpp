#include "../keyvalue.h"
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

class DataReceiverI : public DataReceiver
{
  public:
    virtual void sendData(const DataUnitSeq &dataSeq, const Ice::Current &current) override
    {
        auto endpoint = current.con->getInfo();
        auto tcpInfo = Ice::TCPConnectionInfoPtr::dynamicCast(endpoint);

        if (tcpInfo)
        {
            std::string clientIP = tcpInfo->remoteAddress;
            int clientPort = tcpInfo->remotePort;
            printf("Client IP: %s, Port: %d\n", clientIP.c_str(), clientPort);
        }
        
        json jArray = json::array();

        for (const auto &data : dataSeq)
        {
            json j;
            j["name"] = data.strName;
            j["time"] = data.lTime;
            j["type"] = data.eType;

            switch (data.eType)
            {
            case DSF::ValueType::Decimal:
                j["value"] = data.dValue;
                break;
            case DSF::ValueType::Integer:
                j["value"] = data.lValue;
                break;
            case DSF::ValueType::Boolean:
                j["value"] = data.bValue;
                break;
            case DSF::ValueType::Text:
                j["value"] = data.strValue;
                break;
            default:
                j["value"] = nullptr;
                break;
            }

            jArray.push_back(j);
        }
        std::ofstream file("data.json", std::ios::app); // 使用 std::ios::app 模式打开文件
        file << jArray.dump(4) << std::endl;
        file.close();

        cout << NowMillisecond() << "\nReceived " << dataSeq.size() << " entries and wrote to data.json" << endl;
    }
};

class Server
{
  public:
    int run()
    {
        try
        {
            int argc = 0;
            const char **argv = nullptr;
            Ice::CommunicatorHolder ich(argc, argv);
            // "default -h 127.0.0.1 -p 61235"
            auto adapter =
                ich->createObjectAdapterWithEndpoints("DataReceiverAdapter", "default -h 127.0.0.1 -p 61235");
            Ice::ObjectPtr receiver = new DataReceiverI;
            adapter->add(receiver, Ice::stringToIdentity("DataReceiver"));
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
    server.run();

    return 0;
}
