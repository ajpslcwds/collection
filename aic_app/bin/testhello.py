import requests
import json

# 基础 URL（请根据实际运行的服务地址和端口修改）
BASE_URL = "http://localhost:18080"

def test_hello():
    """测试 GET /hello"""
    url = f"{BASE_URL}/hello"
    try:
        response = requests.get(url)
        response.raise_for_status()
        print("GET /hello ->", response.text)
    except requests.exceptions.RequestException as e:
        print(f"GET /hello 失败: {e}")

def test_greet(name="World"):
    """测试 GET /greet/<string>"""
    url = f"{BASE_URL}/greet/{name}"
    try:
        response = requests.get(url)
        response.raise_for_status()
        print(f"GET /greet/{name} ->", response.text)
    except requests.exceptions.RequestException as e:
        print(f"GET /greet/{name} 失败: {e}")

def test_json_return():
    """测试 GET /json_return（返回 JSON）"""
    url = f"{BASE_URL}/json_return"
    try:
        response = requests.get(url)
        response.raise_for_status()
        data = response.json()  # 自动解析 JSON
        print("GET /json_return ->", json.dumps(data, indent=2, ensure_ascii=False))
    except requests.exceptions.RequestException as e:
        print(f"GET /json_return 失败: {e}")
    except json.JSONDecodeError:
        print("GET /json_return 返回的内容不是合法的 JSON")

def test_post_json(message="Hello from Python"):
    """测试 POST /json，发送 JSON 数据"""
    url = f"{BASE_URL}/json"
    payload = {"message": message}
    headers = {"Content-Type": "application/json"}
    try:
        response = requests.post(url, json=payload, headers=headers)
        # 如果服务端返回的是纯文本，不需要 response.json()
        print(f"POST /json (message='{message}') ->", response.text)
        if response.status_code != 200:
            print(f"  状态码: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"POST /json 失败: {e}")

def main():
    print("=== 测试 Crow RESTful API ===\n")
    test_hello()
    test_greet("Alice")
    test_greet("Bob")
    test_json_return()
    test_post_json("Hello, Crow!")
    test_post_json("")  # 空消息测试
    # 可以添加更多边界测试，如发送非法 JSON
    print("\n=== 测试完成 ===")

if __name__ == "__main__":
    main()