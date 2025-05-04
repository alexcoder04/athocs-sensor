import bme280
import datetime
import os
import requests
import smbus2

from ups import INA219

IP = os.getenv("ATHOCS_IP")
PORT = os.getenv("ATHOCS_PORT")
STATION_ID = os.getenv("ATHOCS_STATION_ID")

address = 0x76
bus = smbus2.SMBus(1)

params = bme280.load_calibration_params(bus, address)

ina219 = INA219(addr=0x43)

data = bme280.sample(bus, address, params)
battery = int((ina219.getBusVoltage_V() - 3.3)/0.9*100)
timestamp = datetime.datetime.now().strftime('%Y-%m-%d_%H:%M:%S')

try:
    requests.post(f"http://{IP}:{PORT}/api/upload", json={
        "timestamp": timestamp,
        "station": STATION_ID,
        "temperature": round(data.temperature, 2),
        "humidity": round(data.humidity, 2),
        "pressure": round(data.pressure, 1),
        "battery": round(battery)
        })
except Exception:
    print(f"{timestamp},{STATION_ID},{data.temperature:.2f},{data.humidity:.2f},{data.pressure:.1f},{battery:d}")

