#include <Ice/Ice.h>
#include "keyvalue.h"
#include <iostream>

class Client
{
public:
    Client(int argc, char* argv[])
    {
        communicator_ = Ice::initialize(argc, argv);

        auto base = communicator_->stringToProxy("receiver:default -h 192.168.30.218 -p 61236");
        receiver_ = Ice::checkedCast<DSF::DataReceiverPrx>(base);

        if (!receiver_)
        {
            throw std::runtime_error("Proxy cast failed");
        }
    }

    void run()
    {
        DSF::DataUnitSeq dataSeq;

        auto unit = std::make_shared<DSF::DataUnit>();
        unit->strName = "sensor";
        unit->lTime = 1718540010000;
        unit->eType = DSF::ValueType::Text;
        unit->strValue = "running";
        unit->dValue = 0.0;
        unit->lValue = 0;
        unit->bValue = false;

        dataSeq.push_back(unit);

        receiver_->sendData(dataSeq);
        std::cout << "Data sent." << std::endl;
    }

    ~Client()
    {
        if (communicator_)
        {
            communicator_->destroy();
        }
    }

private:
    Ice::CommunicatorPtr communicator_;
    DSF::DataReceiverPrxPtr receiver_;  // 修正类型
};

int main(int argc, char* argv[])
{
    try
    {
        Client client(argc, argv);
        client.run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exception: " << ex.what() << std::endl;
        return 1;
    }
}
