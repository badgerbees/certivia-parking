import serial
import time

SERIAL_PORT = "/dev/serial0"
BAUD_RATE = 115200

def open_serial():
    while True:
        try:
            ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            print("Serial connected:", SERIAL_PORT)
            return ser
        except Exception as e:
            print("Waiting for serial...", e)
            time.sleep(2)

ser = open_serial()

print("Listening for ESP32 data...")

while True:
    try:
        line = ser.readline()

        if not line:
            continue

        decoded = line.decode(errors="ignore").strip()
        if decoded:
            print("RECEIVED:", decoded)

    except serial.SerialException as e:
        print("Serial error:", e)
        print("Reconnecting serial...")
        try:
            ser.close()
        except:
            pass
        time.sleep(1)
        ser = open_serial()
