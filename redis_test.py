import redis
import struct
import time
import sys
from datetime import datetime

# 初始化 Redis 连接
r = redis.Redis(host='localhost', port=6380, db=0)

# Redis 中的 key
# key = "STD::DS_TINT"
key = sys.argv[1]

while True:
    value = r.get(key)

    if value is None:
        print(f"❌ Key '{key}' not found.")
    elif len(value) != 10:
        print(f"⚠️ Binary format error: expected 10 bytes, got {len(value)}.")
    else:
        try:
            # 解包为 int16 + int32 + int16 + int16（共10字节）
            int_val, sec, msec, quality = struct.unpack("<hihh", value)
            # 打印一行数据
            print(f"🔢 val: {int_val} | 🕒 sec: {sec} | msec: {msec} | 📶 quality: {quality}" )

        except struct.error as e:
            print(f"❌ Unpack error: {e}")

    time.sleep(0.01)  # 每 10 毫秒（0.01 秒）读取一次
