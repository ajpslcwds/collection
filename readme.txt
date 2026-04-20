目标是 封装spdlog库成为libdsflog.so ，作为日志库组件，给别的业务模块使用，libdsflog.so 对外隐藏spdlog库的符号。
使用third_party  来管理开源库 spdlog  tinyxml2 , 编译之前需要 git submodule update --init

 希望通过配置文件来统一配置多个进程的日志级别、输出新式、大小、是否开启多线程异步等功能
 并且可以试试在线修改这些配置项。

 每个进程init 的时候，指定自己的日志名称 和配置文件的路径。  如果日志名称比如process1 在配置中存在，就用自己的。否则用General