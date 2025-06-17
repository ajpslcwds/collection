rm -rf ./server/server ./client/client keyvalue.h keyvalue.cpp

/home/wzq/code/repos/ice/cpp/install/bin/slice2cpp  keyvalue.ice

g++ -std=c++11 -DICE_CPP11_MAPPING -g -o server/server server/server.cpp server/DataReceiverI.cpp keyvalue.cpp \
    -I. -I/home/wzq/code/repos/ice/cpp/install/include \
    -L/home/wzq/code/repos/ice/cpp/install/lib/x86_64-linux-gnu \
    -lIce++11 -lpthread

g++ -std=c++11 -DICE_CPP11_MAPPING -g -o client/client client/client.cpp keyvalue.cpp \
    -I. -I/home/wzq/code/repos/ice/cpp/install/include \
    -L/home/wzq/code/repos/ice/cpp/install/lib/x86_64-linux-gnu \
    -lIce++11 -lpthread
