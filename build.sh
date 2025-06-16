
/home/wzq/code/repos/ice/cpp/install/bin/slice2cpp keyvalue.ice

g++ -std=c++17 -o server/server     server/server.cpp server/DataReceiverI.cpp keyvalue.cpp   -I. -I/home/wzq/code/repos/ice/cpp/install/include -L/home/wzq/code/repos/ice/cpp/install/lib/x86_64-linux-gnu -lIce   -lpthread

g++ -std=c++17 -o client/client client/client.cpp  keyvalue.cpp   -I. -I/home/wzq/code/repos/ice/cpp/install/include -L/home/wzq/code/repos/ice/cpp/install/lib/x86_64-linux-gnu -lIce   -lpthread
