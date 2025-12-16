#!/usr/bin/env python3
"""
Certivia Parking - Raspberry Pi Gateway
Reads sensor data from ESP32 via serial and sends to backend API

Expected serial format from ESP32:
  A1:0:250  (slot_id:state:distance)
  
  state: 0 = empty, 1 = occupied
  distance: 0-250 cm (ultrasonic sensor reading)
"""

import serial
import requests
import time
import sys
from datetime import datetime

# Configuration
BACKEND_URL = "https://api.parking.certivia.id"  # Vercel backend
DEVICE_KEY = "PI_GATEWAY_01"  # Must match backend DEVICE_KEY
REQUEST_TIMEOUT = 3  # seconds

# Serial port settings
SERIAL_PORT = "/dev/serial0"  # Change if your ESP32 is on different port (e.g., /dev/ttyUSB1, /dev/ttyAMA0)
BAUD_RATE = 115200

# Reconnection settings
RECONNECT_DELAY = 5  # seconds
SEND_INTERVAL = 2  # seconds between updates

def send_sensor_update(slot_id: str, state: int, distance: int) -> bool:
    """
    Send a sensor update to the backend.
    
    Args:
        slot_id: Parking slot ID (e.g., "A1", "B3")
        state: 0 = empty, 1 = occupied
        distance: Distance reading in cm (0-250)
    
    Returns:
        True if successful, False otherwise
    """
    try:
        payload = {
            "slot_id": slot_id,
            "state": state,
            "distance": distance,
            "device_key": DEVICE_KEY
        }
        
        response = requests.post(
            f"{BACKEND_URL}/api/parking/update",
            json=payload,
            timeout=REQUEST_TIMEOUT
        )
        
        if response.status_code == 200:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] ✓ {slot_id}: state={state}, distance={distance}cm")
            return True
        else:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] ✗ {slot_id}: HTTP {response.status_code}")
            return False
            
    except requests.exceptions.Timeout:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] ✗ {slot_id}: Request timeout")
        return False
    except requests.exceptions.ConnectionError:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] ✗ {slot_id}: Backend unreachable")
        return False
    except Exception as e:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] ✗ {slot_id}: {e}")
        return False


def connect_serial(port: str, baud: int) -> serial.Serial:
    """Establish serial connection to ESP32"""
    try:
        ser = serial.Serial(port, baud, timeout=1)
        print(f"[{datetime.now().strftime('%H:%M:%S')}] ✓ Connected to {port} at {baud} baud")
        return ser
    except serial.SerialException as e:
        print(f"[{datetime.now().strftime('%H:%M:%S')}] ✗ Serial error: {e}")
        return None


def parse_sensor_data(line: str) -> tuple:
    """
    Parse serial data from ESP32.
    Expected format: "A1:0:250" (slot_id:state:distance)
    
    Returns:
        (slot_id, state, distance) or (None, None, None) if parse error
    """
    try:
        parts = line.strip().split(':')
        if len(parts) != 3:
            return None, None, None
        
        slot_id = parts[0].strip()
        state = int(parts[1].strip())
        distance = int(parts[2].strip())
        
        # Validate values
        if state not in [0, 1]:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] ⚠ Invalid state: {state}")
            return None, None, None
        if distance < 0 or distance > 250:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] ⚠ Invalid distance: {distance}cm")
            return None, None, None
        
        return slot_id, state, distance
    except ValueError:
        return None, None, None


def main():
    """Main loop - read from ESP32 and send to backend"""
    print("=" * 70)
    print("Certivia Parking - Raspberry Pi Gateway")
    print(f"Backend: {BACKEND_URL}")
    print(f"Device Key: {DEVICE_KEY}")
    print(f"Serial Port: {SERIAL_PORT} @ {BAUD_RATE} baud")
    print(f"Expected format from ESP32: SLOT_ID:STATE:DISTANCE")
    print(f"  Example: A1:0:250 (slot A1, empty, 250cm)")
    print("=" * 70)
    
    ser = None
    last_send_time = 0
    
    while True:
        # Connect to serial if not connected
        if ser is None or not ser.is_open:
            print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Attempting to connect...")
            ser = connect_serial(SERIAL_PORT, BAUD_RATE)
            if ser is None:
                print(f"[{datetime.now().strftime('%H:%M:%S')}] Retrying in {RECONNECT_DELAY}s...")
                time.sleep(RECONNECT_DELAY)
                continue
        
        try:
            # Read line from ESP32
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='ignore')
                
                if line.strip():
                    print(f"[{datetime.now().strftime('%H:%M:%S')}] RX: {line.strip()}")
                    
                    # Parse data
                    slot_id, state, distance = parse_sensor_data(line)
                    
                    if slot_id and state is not None and distance is not None:
                        # Rate limit to avoid spamming backend
                        current_time = time.time()
                        if current_time - last_send_time >= SEND_INTERVAL:
                            send_sensor_update(slot_id, state, distance)
                            last_send_time = current_time
                        else:
                            print(f"[{datetime.now().strftime('%H:%M:%S')}] (rate limited, waiting...)")
                    else:
                        print(f"[{datetime.now().strftime('%H:%M:%S')}] ⚠ Parse error: {line.strip()}")
            
            time.sleep(0.1)  # Prevent CPU spinning
            
        except serial.SerialException:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] ✗ Serial connection lost")
            if ser:
                ser.close()
            ser = None
        except KeyboardInterrupt:
            print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Shutdown requested")
            break
        except Exception as e:
            print(f"[{datetime.now().strftime('%H:%M:%S')}] ✗ Error: {e}")
            time.sleep(1)
    
    if ser:
        ser.close()
    print("Goodbye!")


if __name__ == "__main__":
    main()
