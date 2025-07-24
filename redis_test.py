import redis
import struct

# 连接 Redis 服务器（以二进制模式，不解码）
r = redis.Redis(host='localhost', port=6380, decode_responses=False)

# 开启 pipeline（非事务模式）
pipe = r.pipeline(transaction=False)

# 批量写入 100 个键值对，键为 b'mykey_1' 到 b'mykey_100'，值为 int 的二进制表示
for i in range(1, 101):
    key = f"mykey_{i}".encode('utf-8')  # 将字符串转换为字节
    value = struct.pack(">i", i)        # 将整数打包为4字节二进制（大端序）
    pipe.set(key, value)

# 执行所有命令
results = pipe.execute()

# 输出结果
for i, res in enumerate(results):
    print(f"Reply {i}: {res}")