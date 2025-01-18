import smbus2
import bme280
import time
import datetime
import requests

IP = "192.168.0.86"
PORT = 3000
STATION_ID = "RPIZ-ALEX"
INTERVAL = 60 * 5

address = 0x76
bus = smbus2.SMBus(1)

params = bme280.load_calibration_params(bus, address)

while True:
    data = bme280.sample(bus, address, params)
    timestamp = datetime.datetime.now().strftime('%Y-%m-%d_%H:%M:%S')

    try:
        requests.post(f"http://{IP}:{PORT}/api/upload", json={
            "timestamp": timestamp,
            "temperature": round(data.temperature, 2),
            "humidity": round(data.humidity, 2),
            "pressure": round(data.pressure, 1),
            "station": STATION_ID,
            "battery": 100
            })
    except Exception:
        print("failed to upload data")

    time.sleep(INTERVAL)

