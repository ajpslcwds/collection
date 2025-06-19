rm -rf keyvalue.cpp keyvalue.h server/server  client/client

/home/wzq/code/repos/ice/cpp/install/bin/slice2cpp keyvalue.ice

g++ -g -o server/server server/server.cpp  keyvalue.cpp \
    -I. -I/home/wzq/code/repos/ice/cpp/install/include \
    -L/home/wzq/code/repos/ice/cpp/install/lib/x86_64-linux-gnu \
    -lIce -lpthread

g++ -g -o client/client client/client.cpp keyvalue.cpp \
    -I. -I/home/wzq/code/repos/ice/cpp/install/include \
    -L/home/wzq/code/repos/ice/cpp/install/lib/x86_64-linux-gnu \
    -lIce -lpthread