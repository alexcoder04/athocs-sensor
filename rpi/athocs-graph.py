import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime, timedelta
import matplotlib.dates as mdates

INPUT_FILE = "/home/alex/web/athocs.1.log.csv"
OUTPUT_FILE = "/home/alex/web/graph-last-24-hours.png"

# Load the CSV file
data = pd.read_csv(INPUT_FILE)

# Ensure the datetime column is in datetime format
data["datetime"] = pd.to_datetime(data["datetime"])

# Get the current time and calculate the time 24 hours ago
now = datetime.now()
twenty_four_hours_ago = now - timedelta(hours=24)

# Filter the data for the last 24 hours
last_24_hours_data = data[data["datetime"] >= twenty_four_hours_ago]

# Create a figure and axis for the plot
fig, ax1 = plt.subplots(figsize=(12, 6))

# Plot temperature on the first y-axis
ax1.plot(last_24_hours_data["datetime"], last_24_hours_data["temperature"], label="Temperature (°C)", color="red")
ax1.set_xlabel("Datetime")
ax1.set_ylabel("Temperature (°C)", color="red")
ax1.tick_params(axis="y", labelcolor="red")

# Create a second y-axis for humidity
ax2 = ax1.twinx()
ax2.plot(last_24_hours_data["datetime"], last_24_hours_data["humidity"], label="Humidity (%)", color="blue")
ax2.set_ylabel("Humidity (%)", color="blue")
ax2.tick_params(axis="y", labelcolor="blue")

# Create a third y-axis for pressure
ax3 = ax1.twinx()
ax3.spines["right"].set_position(("outward", 60))  # Offset the third axis
ax3.plot(last_24_hours_data["datetime"], last_24_hours_data["pressure"], label="Pressure (hPa)", color="green", alpha=0.25)
ax3.set_ylabel("Pressure (hPa)", color="green")
ax3.tick_params(axis="y", labelcolor="green")

# Format the x-axis to show month-date hour:minute and display every hour
ax1.xaxis.set_major_formatter(mdates.DateFormatter("%m-%d %H:%M"))
ax1.xaxis.set_major_locator(mdates.HourLocator(interval=1))
ax1.tick_params("x", rotation=45)
#plt.xticks(rotation=90)

# Add a title and adjust layout
plt.title("Temperature, Humidity, and Pressure in the Last 24 Hours")
fig.tight_layout()

# Save the plot as an image
plt.savefig(OUTPUT_FILE)

