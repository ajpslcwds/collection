#include "DataReceiverI.h"
#include <Ice/Ice.h>
#include <memory>

int main(int argc, char *argv[])
{
    try
    {
        Ice::CommunicatorHolder ich(argc, argv);
        auto adapter = ich->createObjectAdapterWithEndpoints("DataReceiverAdapter", "default -p 61236");

        // auto servant = std::make_shared<DataReceiverI>();
        // adapter->add(Ice::ObjectPtr(servant), Ice::stringToIdentity("receiver"));
        adapter->add(new DataReceiverI, Ice::stringToIdentity("receiver"));

        adapter->activate();
        std::cout << "Server started." << std::endl;
        ich->waitForShutdown();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}