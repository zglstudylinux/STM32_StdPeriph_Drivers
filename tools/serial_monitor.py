#!/usr/bin/env python3
"""
STM32串口监控工具
功能：
1. 读取STM32串口输出
2. 显示调试信息
"""

import serial
import threading
import time
import sys

def serial_monitor(port='COM3', baudrate=115200):
    """串口监控主函数"""
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        print(f"Serial monitor started on {port} at {baudrate} baud")
        print("Press Ctrl+C to exit...\n")
        
        def read_serial():
            while True:
                try:
                    if ser.in_waiting > 0:
                        data = ser.readline().decode('utf-8', errors='ignore')
                        if data:
                            print(data, end='')
                    time.sleep(0.01)
                except Exception as e:
                    print(f"Read error: {e}")
                    break
        
        read_thread = threading.Thread(target=read_serial)
        read_thread.daemon = True
        read_thread.start()
        
        while True:
            time.sleep(1)
            
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        print("Available ports:")
        if sys.platform.startswith('win'):
            for i in range(20):
                try:
                    ser = serial.Serial(f'COM{i+1}')
                    ser.close()
                    print(f"  COM{i+1}")
                except:
                    pass

if __name__ == "__main__":
    serial_monitor()