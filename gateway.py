import serial
import requests
import time

# ===============================
# CONFIGURATION
# ===============================

SERIAL_PORT = "/dev/ttyS0"    # UART from ESP-WROOM
BAUD_RATE = 115200

API_URL = "https://parking.certivia.id/api/parking/update"

# ===============================
# START SERIAL
# ===============================

while True:
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print("Serial connected to ESP-WROOM")
        break
    except Exception as e:
        print("Serial error:", e)
        time.sleep(2)

# ===============================
# MAIN LOOP
# ===============================

print("Gateway running...")

while True:
    try:
        line = ser.readline().decode().strip()

        if not line:
            continue

        # expected format: "82,1"
        try:
            distance_str, occupied_str = line.split(",")
            distance = int(distance_str)
            occupied = int(occupied_str)

        except ValueError:
            print("Malformed packet:", line)
            continue

        # package into JSON
        payload = {
            "slot_id": 1,          # change if you want per-node slot IDs
            "distance": distance,
            "occupied": bool(occupied)
        }

        # send to backend
        try:
            r = requests.post(API_URL, json=payload, timeout=3)
            print("Sent →", payload, "| Response:", r.status_code)

        except requests.exceptions.RequestException as e:
            print("HTTP error:", e)

    except Exception as e:
        print("Loop error:", e)
        time.sleep(1)
