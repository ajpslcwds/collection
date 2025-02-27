import socket

# 创建一个 TCP 套接字
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# 获取本地主机名
host = '127.0.0.1'  # 本地地址
port = 18080  # 设置监听端口

# 绑定套接字到指定的 IP 地址和端口
server_socket.bind((host, port))

# 启动监听，最多允许 5 个连接
server_socket.listen(5)
print(f"Server listening on {host}:{port}...")

# 等待连接
while True:
    # 接受连接，返回连接对象和客户端的地址
    client_socket, addr = server_socket.accept()
    print(f"Connection from {addr} has been established!")

    # 接收客户端发送的数据
    data = client_socket.recv(1024)
    print(f"Received data: {data.decode('utf-8')}")

    # 给客户端发送响应数据
    message = "Hello, client!"
    client_socket.send(message.encode('utf-8'))

    # 关闭与客户端的连接
    client_socket.close()

