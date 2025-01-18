import smbus2
import bme280
import datetime
import requests

IP = "192.168.0.86"
PORT = 3000
STATION_ID = "RPIZ-ALEX"

address = 0x76
bus = smbus2.SMBus(1)

params = bme280.load_calibration_params(bus, address)

data = bme280.sample(bus, address, params)
timestamp = datetime.datetime.now().strftime('%Y-%m-%d_%H:%M:%S')

try:
    requests.post(f"http://{IP}:{PORT}/api/upload", json={
        "timestamp": timestamp,
        "station": STATION_ID,
        "temperature": round(data.temperature, 2),
        "humidity": round(data.humidity, 2),
        "pressure": round(data.pressure, 1),
        "battery": 100
        })
except Exception:
    print(f"{timestamp},{STATION_ID},{temperature:.2f},{humidity:.2f},{pressure:.1f},100")

