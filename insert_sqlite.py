import sqlite3
import random
import json

DB_PATH = "/home/wzq/code/repos/dbswitch/T_DD_SM_VAR.db"
BASE = 0
TOTAL = 200000
BATCH = 5000

conn = sqlite3.connect(DB_PATH)
cursor = conn.cursor()

def random_json():
    return json.dumps({
        "ELE_TYPE": random.choice(["INT","FLOAT","BOOL"]),
        "STR_LENGTH": random.randint(1,256),
        "MODEL_ID": random.randint(1,10),
        "RANGE":[0,random.randint(10,100)]
    })

insert_sql = """
INSERT INTO T_DD_SM_VAR
(TAG_NAME,NODE_ID,NAMESPACE,TYPE_ID,TYPE_EXTRA,IO_ADDR,SCAN_INTV,DEVICE_ID)
VALUES (?,?,?,?,?,?,?,?)
"""

data = []

for i in range(BASE, BASE+TOTAL+1):
    row = (
        f"TAG_{i}",
        i%3+1,
        "STD",
        i%10,
        random_json(),
        f"STD::TAG_{i}",
        500,
        i%5+1
    )

    data.append(row)

    if len(data) >= BATCH:
        cursor.executemany(insert_sql, data)
        data.clear()

if data:
    cursor.executemany(insert_sql, data)

conn.commit()
conn.close()

print(f"Inserted {TOTAL} rows.")