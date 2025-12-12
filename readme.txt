目标是 封装spdlog库成为libdsflog.so ，作为日志库组件，给别的业务模块使用，libdsflog.so 对外隐藏spdlog库的符号。
使用third_party  来管理开源库 spdlog  tinyxml2 , 编译之前需要 git submodule update --init


 