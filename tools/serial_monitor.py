#!/usr/bin/env python3
"""
STM32 Serial Monitor Tool
Usage: python serial_monitor.py [PORT] [BAUDRATE]
  Default: COM3 @ 115200

Examples:
  python serial_monitor.py                # Use COM3 @ 115200
  python serial_monitor.py COM5           # Use COM5 @ 115200
  python serial_monitor.py COM5 9600      # Use COM5 @ 9600
  python serial_monitor.py --list         # List available ports
"""

import serial
import serial.tools.list_ports
import sys
import threading
import time


def list_ports():
    """List all available serial ports."""
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("No serial ports found!")
    else:
        print("Available serial ports:")
        for port in sorted(ports):
            print(f"  {port.device} - {port.description}")


def serial_monitor(port='COM3', baudrate=115200):
    """Monitor a serial port and print received data."""
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"[Serial Monitor] Connected to {port} @ {baudrate} baud")
        print("[Serial Monitor] Press Ctrl+C to exit...\n")

        def read_serial():
            while True:
                try:
                    if ser.in_waiting > 0:
                        data = ser.read(ser.in_waiting)
                        # Try UTF-8 first, fall back to replacing errors
                        text = data.decode('utf-8', errors='replace')
                        print(text, end='', flush=True)
                    time.sleep(0.01)
                except Exception as e:
                    print(f"\n[Error] Read error: {e}")
                    break

        read_thread = threading.Thread(target=read_serial, daemon=True)
        read_thread.start()

        # Keep main thread alive
        while True:
            time.sleep(1)

    except serial.SerialException as e:
        print(f"[Error] Cannot open {port}: {e}")
        list_ports()
        sys.exit(1)
    except KeyboardInterrupt:
        print("\n[Serial Monitor] Stopped by user.")
        ser.close()


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] in ('--list', '-l', 'list'):
        list_ports()
        sys.exit(0)

    port = sys.argv[1] if len(sys.argv) > 1 else 'COM3'
    try:
        baudrate = int(sys.argv[2]) if len(sys.argv) > 2 else 115200
    except ValueError:
        print(f"Invalid baudrate: {sys.argv[2]}")
        sys.exit(1)

    serial_monitor(port, baudrate)
