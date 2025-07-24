import redis
import time

# 连接 Redis 服务器（确保 Redis 已启动，端口正确）
r = redis.Redis(host='localhost', port=6380, decode_responses=True)

while True:
    try:
        pipe = r.pipeline(transaction=False)
        pipe.get("mykey")              # 向 pipeline 中添加一个 GET 命令
        result = pipe.execute()        # 发送并执行 pipeline
        if result[0]!="3":
            print(f"mykey: {result[0]}")   # 打印返回值
    except redis.exceptions.ConnectionError as e:
        print(f"Redis 连接失败: {e}")
        break
    except Exception as e:
        print(f"发生异常: {e}")
        break

    time.sleep(0.001)  # 每秒钟读取一次
