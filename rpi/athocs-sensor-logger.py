import smbus2
import bme280
import time
import datetime

LOG_FILE = "/var/log/athocs.1.log"
INTERVAL = 60 * 5

address = 0x76
bus = smbus2.SMBus(1)

params = bme280.load_calibration_params(bus, address)

with open(LOG_FILE, "a") as f:
    while True:
        data = bme280.sample(bus, address, params)
        f.write(f"{datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')},{data.temperature:.2f},{data.humidity:.1f},{data.pressure:.0f}\n")
        time.sleep(INTERVAL)

