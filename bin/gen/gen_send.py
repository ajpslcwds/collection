
import json
import copy
import glob
import os

SEND_TEMPLATE = "/home/wzq/code/DSF_ubuntu_amd64/bin/gen/cmd_send.dsfngvs"
RECV_TEMPLATE = "/home/wzq/code/DSF_ubuntu_amd64/bin/gen/cmd_recv.dsfngvs"
SEND_DIR = "/home/wzq/code/DSF_ubuntu_amd64/bin/send/NGVS"
RECV_DIR = "/home/wzq/code/DSF_ubuntu_amd64/bin/recv/NGVS"

# ===== 开头清空旧文件 =====
for path in glob.glob(os.path.join(SEND_DIR, "*")):
    if os.path.isfile(path):
        os.remove(path)

for path in glob.glob(os.path.join(RECV_DIR, "*")):
    if os.path.isfile(path):
        os.remove(path)


COUNT =192

# 读取模板
with open(SEND_TEMPLATE, "r", encoding="utf-8") as f:
    send_tpl = json.load(f)

with open(RECV_TEMPLATE, "r", encoding="utf-8") as f:
    recv_tpl = json.load(f)


for i in range(1, COUNT + 1):
    # ===== 生成 Send =====
    send_data = copy.deepcopy(send_tpl)
    send_data["SendNGVS"][0]["Name"] = f"cmd_send{i}"
    send_data["SendNGVS"][0]["LocalIPAddress"] = "192.168.30.218"
    send_data["SendNGVS"][0]["LocalPort"] = 8000 + i
    send_data["SendNGVS"][0]["Protocol"] = 0
 

    send_file = f"{SEND_DIR}/cmd_send{i}.dsfngvs"
    with open(send_file, "w", encoding="utf-8") as f:
        json.dump(send_data, f, indent=4, ensure_ascii=False)

    # ===== 生成 Recv =====        
    recv_data = copy.deepcopy(recv_tpl)
    recv_data["RecvNGVS"][0]["Name"] = f"cmd_recv{i}"
    recv_data["RecvNGVS"][0]["SendNGVSName"] = f"cmd_send{i}"
    recv_data["RecvNGVS"][0]["SendIPAddress"] = "192.168.30.218"
    recv_data["RecvNGVS"][0]["SendPort"] = 8000 + i
    recv_data["RecvNGVS"][0]["LocalIPAddress"] = "192.168.30.218"
    recv_data["RecvNGVS"][0]["LocalPort"] = 9000 
    recv_data["RecvNGVS"][0]["Protocol"] = 0


    recv_file = f"{RECV_DIR}/cmd_recv{i}.dsfngvs"
    with open(recv_file, "w", encoding="utf-8") as f:
        json.dump(recv_data, f, indent=4, ensure_ascii=False)

print("已生成 cmd_sendX / cmd_recvX dsfngvs 文件")
