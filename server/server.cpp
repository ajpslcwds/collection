#include "DataReceiverI.h"
#include <Ice/Ice.h>
#include <memory>

int main(int argc, char *argv[])
{
    try
    {
        Ice::CommunicatorHolder ich(argc, argv);
        auto adapter = ich->createObjectAdapterWithEndpoints("DataReceiverAdapter", "default -p 61236");

        auto servant = std::make_shared<DataReceiverI>();
        adapter->add(Ice::ObjectPtr(servant), Ice::stringToIdentity("receiver"));
        adapter->activate();
        std::cout << "Server started." << std::endl;
        ich->waitForShutdown();
    }
    catch (const Ice::Exception &ex)
    {
        std::cerr << "Ice exception: " << ex.what() << std::endl;
        return 1;
    }
    catch (const std::exception &ex)
    {
        std::cerr << "std exception: " << ex.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "unknown exception" << std::endl;
        return 1;
    }

    return 0;
}