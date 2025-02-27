import requests
import time

url = "http://127.0.0.1:18080/restapi/set"
headers = {"Content-Type": "application/json"}
params = {"name": "C612_aic_spm_review"}
data = {"key": "value"}

response = requests.post(url, headers=headers, params=params, json=data)
print("Status Code:", response.status_code)
print("Response Body:", response.text)

time.sleep(1)

url = "http://127.0.0.1:18080/restapi/set"
headers = {"Content-Type": "application/json"}
params = {"name": "C612_aic_spm_result"}
data = {"key": "value"}

response = requests.post(url, headers=headers, params=params, json=data)
print("Status Code:", response.status_code)
print("Response Body:", response.text)

