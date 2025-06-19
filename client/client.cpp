#include "keyvalue.h"
#include <Ice/Ice.h>
#include <chrono>

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

    int sendData(const DSF::DataUnitSeq &seq);

  private:
    Ice::CommunicatorHolder _communicator;
    DSF::DataReceiverPrx _receiver;
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
        auto base = _communicator->stringToProxy("DSF/DataReceiver:default -h 127.0.0.1 -p 55010");
        _receiver = DSF::DataReceiverPrx::checkedCast(base);
        if (!_receiver)
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

int IceClient::sendData(const DSF::DataUnitSeq &seq)
{
    try
    {
        _receiver->sendData(seq);
        return 0;
    }
    ICE_DRV_CATCH
}

inline uint64_t NowMillisecond()
{
    using namespace std::chrono;
    auto now = system_clock::now();

    auto milliseconds_since_epoch = duration_cast<milliseconds>(now.time_since_epoch()).count();

    return static_cast<uint64_t>(milliseconds_since_epoch);
}
int main(int argc, char *argv[])
{
    try
    {
        IceClient client;
        client.Connect();

        DSF::DataUnitSeq seq;
        {
            DSF::DataUnit d;
            d.strName = "ICE_LINT";
            d.lTime = NowMillisecond();
            d.eType = DSF::ValueType::Integer;
            d.lValue = 123;
            seq.push_back(d);
        }
        {
            DSF::DataUnit d;
            d.strName = "ICE_LREAL";
            d.lTime = NowMillisecond();
            d.eType = DSF::ValueType::Decimal;
            d.dValue = 22.5;
            seq.push_back(d);
        }
        {
            DSF::DataUnit d;
            d.strName = "ICE_BOOL";
            d.lTime = 1718540011000;
            d.eType = DSF::ValueType::Boolean;
            d.bValue = true;
            seq.push_back(d);
        }
        {
            DSF::DataUnit d;
            d.strName = "ice_string";
            d.lTime = 1718540012000;
            d.eType = DSF::ValueType::Text;
            d.strValue = "dsf";
            seq.push_back(d);
        }

        client.sendData(seq);
        std::cout << "Data sent." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}