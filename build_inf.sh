rm -rf dsf_ice_inf.cpp dsf_ice_inf.h server/server_inf  client/client_inf

/home/wzq/code/repos/ice/cpp/install/bin/slice2cpp -I/home/wzq/code/repos/ice/slice/  dsf_ice_inf.ice 

g++ -g -o server/server_inf server/server_inf.cpp  dsf_ice_inf.cpp \
    -I. -I/home/wzq/code/repos/ice/cpp/install/include \
    -L/home/wzq/code/repos/ice/cpp/install/lib/x86_64-linux-gnu \
    -lIce -lpthread

g++ -g -o client/client_inf client/client_inf.cpp dsf_ice_inf.cpp \
    -I. -I/home/wzq/code/repos/ice/cpp/install/include \
    -L/home/wzq/code/repos/ice/cpp/install/lib/x86_64-linux-gnu \
    -lIce -lpthread