#include "keyvalue.h"
#include <Ice/Ice.h>
#include <iostream>

int main(int argc, char *argv[])
{
    try
    {
        Ice::CommunicatorHolder ich(argc, argv);
        auto base = ich->stringToProxy("receiver:default   -h 192.168.30.218 -p 61236");

        auto receiver = DSF::DataReceiverPrx::checkedCast(base);
        if (!receiver)
        {
            std::cerr << "Invalid proxy" << std::endl;
            return 1;
        }

        DSF::DataUnitSeq seq;

        {
            auto d = new DSF::DataUnit;
            d->strName = "temperature";
            d->lTime = 1718540010000;
            d->eType = DSF::ValueType::Decimal;
            d->dValue = 22.5;
            seq.push_back(d);
        }

        {
            auto d = new DSF::DataUnit;
            d->strName = "device_online";
            d->lTime = 1718540011000;
            d->eType = DSF::ValueType::Bool;
            d->bValue = true;
            seq.push_back(d);
        }

        {
            auto d = new DSF::DataUnit;
            d->strName = "comment";
            d->lTime = 1718540012000;
            d->eType = DSF::ValueType::Text;
            d->strValue = "sensor ready";
            seq.push_back(d);
        }

        receiver->sendData(seq);
        std::cout << "Sent " << seq.size() << " items." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    return 0;
}
