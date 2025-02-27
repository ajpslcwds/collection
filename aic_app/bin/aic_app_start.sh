#!/bin/bash

PORT=18080
LOG_FILE="aic_app.log"
APP="./aic_app"
CONFIG="./config.json"

# 获取脚本所在目录
SCRIPT_DIR=$(dirname "$(realpath "$0")")

echo "脚本所在目录: $SCRIPT_DIR"
cd $SCRIPT_DIR

# 循环检查端口是否被占用
while true; do
    # 使用 netstat 检查端口是否被占用
    if ! netstat -anlp | grep -q ":$PORT "; then
        echo "端口 $PORT 未被占用，启动 aic_app..."

        # 使用 nohup 启动应用程序并将日志输出到 aic_app.log
        nohup $APP $CONFIG >$LOG_FILE 2>&1 &

        # 等待应用程序启动并检查是否启动成功
        echo "aic_app 已启动，日志输出到 $LOG_FILE"

        # 启动后退出循环
        break
    else
        echo "端口 $PORT 已被占用，继续检查..."
    fi

    # 等待 5 秒后再次检查
    sleep 3
done
