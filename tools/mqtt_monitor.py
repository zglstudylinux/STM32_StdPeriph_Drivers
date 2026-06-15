#!/usr/bin/env python3
"""
STM32温湿度监控上位机
功能：
1. TCP服务器接收ESP8266发送的JSON数据
2. 显示温湿度数据
3. 简单的GUI界面
"""

import tkinter as tk
from tkinter import ttk
import json
import threading
import time
from datetime import datetime
import socket

TCP_PORT = 1883

sensor_data = {
    "temperature": 0,
    "humidity": 0,
    "timestamp": "",
    "count": 0,
    "alarm": 0,
    "led_blink": 0
}

class TCPServer:
    def __init__(self, port=1883):
        self.port = port
        self.running = False
        self.server_socket = None
        
    def start(self):
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.server_socket.bind(('0.0.0.0', self.port))
        self.server_socket.listen(5)
        self.running = True
        print(f"TCP Server started on port {self.port}")
        
        accept_thread = threading.Thread(target=self._accept_connections)
        accept_thread.daemon = True
        accept_thread.start()
        
    def _accept_connections(self):
        while self.running:
            try:
                self.server_socket.settimeout(1.0)
                client_socket, address = self.server_socket.accept()
                print(f"Client connected from {address}")
                client_thread = threading.Thread(target=self._handle_client, args=(client_socket, address))
                client_thread.daemon = True
                client_thread.start()
            except socket.timeout:
                continue
            except Exception as e:
                if self.running:
                    print(f"Accept error: {e}")
                    
    def _handle_client(self, client_socket, address):
        try:
            while self.running:
                try:
                    client_socket.settimeout(5.0)
                    data = client_socket.recv(1024)
                    if not data:
                        break
                    
                    self._parse_json(data)
                    
                except socket.timeout:
                    continue
                except Exception as e:
                    print(f"Receive error: {e}")
                    break
                    
        except Exception as e:
            print(f"Client handler error: {e}")
        finally:
            client_socket.close()
            print(f"Client {address} disconnected")
            
    def _parse_json(self, data):
        try:
            json_start = data.find(b'{')
            if json_start >= 0:
                json_data = data[json_start:]
                json_end = json_data.find(b'}')
                if json_end >= 0:
                    json_str = json_data[:json_end+1].decode('utf-8')
                    payload = json.loads(json_str)
                    
                    global sensor_data
                    sensor_data["temperature"] = payload.get("temperature", 0)
                    sensor_data["humidity"] = payload.get("humidity", 0)
                    sensor_data["timestamp"] = datetime.now().strftime("%H:%M:%S")
                    sensor_data["count"] = payload.get("count", 0)
                    sensor_data["alarm"] = payload.get("alarm", 0)
                    sensor_data["led_blink"] = payload.get("led_blink", 0)

                    alarm_str = "⚠ ALARM!" if sensor_data["alarm"] else "OK"
                    blink_str = "↯ 闪烁" if sensor_data["led_blink"] else "● 常亮"
                    print(f"Received: Temp={sensor_data['temperature']}C, Humid={sensor_data['humidity']}%, Count={sensor_data['count']}, Alarm={alarm_str}, LED={blink_str}")
                    
        except Exception as e:
            print(f"Parse error: {e}: {data}")
            
    def stop(self):
        self.running = False
        if self.server_socket:
            self.server_socket.close()


class SensorMonitorGUI:
    def __init__(self):
        self.root = tk.Tk()
        self.root.title("STM32温湿度监控")
        self.root.geometry("520x480")
        self.root.resizable(True, True)
        self.root.minsize(440, 400)
        
        self.server = TCPServer(TCP_PORT)
        self.setup_ui()
        
    def setup_ui(self):
        title_label = ttk.Label(self.root, text="STM32温湿度监控系统", font=("Arial", 18, "bold"))
        title_label.pack(pady=12)

        status_frame = ttk.LabelFrame(self.root, text="连接状态")
        status_frame.pack(fill="x", padx=15, pady=6)

        self.status_label = ttk.Label(status_frame, text="TCP Server: 运行中 (端口: 1883)", foreground="green", font=("Arial", 11))
        self.status_label.pack(pady=8)

        data_frame = ttk.LabelFrame(self.root, text="传感器数据")
        data_frame.pack(fill="x", padx=15, pady=6)

        temp_frame = ttk.Frame(data_frame)
        temp_frame.pack(fill="x", pady=6)

        ttk.Label(temp_frame, text="温度:", font=("Arial", 14)).pack(side="left", padx=12)
        self.temp_label = ttk.Label(temp_frame, text="-- C", font=("Arial", 18, "bold"), foreground="blue")
        self.temp_label.pack(side="left", padx=12)

        humid_frame = ttk.Frame(data_frame)
        humid_frame.pack(fill="x", pady=6)

        ttk.Label(humid_frame, text="湿度:", font=("Arial", 14)).pack(side="left", padx=12)
        self.humid_label = ttk.Label(humid_frame, text="-- %", font=("Arial", 18, "bold"), foreground="green")
        self.humid_label.pack(side="left", padx=12)

        info_frame = ttk.Frame(data_frame)
        info_frame.pack(fill="x", pady=6)

        ttk.Label(info_frame, text="更新时间:", font=("Arial", 11)).pack(side="left", padx=12)
        self.time_label = ttk.Label(info_frame, text="--:--:--", font=("Arial", 11))
        self.time_label.pack(side="left", padx=6)

        ttk.Label(info_frame, text="计数:", font=("Arial", 11)).pack(side="left", padx=12)
        self.count_label = ttk.Label(info_frame, text="0", font=("Arial", 11))
        self.count_label.pack(side="left", padx=6)

        # 报警状态
        alarm_frame = ttk.LabelFrame(self.root, text="报警状态")
        alarm_frame.pack(fill="x", padx=15, pady=6)

        alarm_info = ttk.Frame(alarm_frame)
        alarm_info.pack(fill="x", pady=10)

        ttk.Label(alarm_info, text="报警:", font=("Arial", 14)).pack(side="left", padx=12)
        self.alarm_label = ttk.Label(alarm_info, text="--", font=("Arial", 18, "bold"))
        self.alarm_label.pack(side="left", padx=12)

        ttk.Label(alarm_info, text="LED:", font=("Arial", 14)).pack(side="left", padx=24)
        self.led_label = ttk.Label(alarm_info, text="--", font=("Arial", 18, "bold"))
        self.led_label.pack(side="left", padx=12)

        button_frame = ttk.Frame(self.root)
        button_frame.pack(fill="x", padx=10, pady=10)
        
        self.start_button = ttk.Button(button_frame, text="启动服务器", command=self.start_server)
        self.start_button.pack(side="left", padx=5)
        
        self.stop_button = ttk.Button(button_frame, text="停止服务器", command=self.stop_server)
        self.stop_button.pack(side="left", padx=5)
        
        self.start_server()
        
    def start_server(self):
        if not self.server.running:
            self.server.start()
            self.status_label.config(text="TCP Server: 运行中 (端口: 1883)", foreground="green")
            
    def stop_server(self):
        self.server.stop()
        self.status_label.config(text="TCP Server: 已停止", foreground="red")
        
    def update_display(self):
        self.temp_label.config(text=f"{sensor_data['temperature']} C")
        self.humid_label.config(text=f"{sensor_data['humidity']} %")
        self.time_label.config(text=sensor_data['timestamp'])
        self.count_label.config(text=str(sensor_data['count']))

        # 报警状态
        if sensor_data.get("alarm", 0):
            self.alarm_label.config(text="⚠ 报警中!", foreground="red")
        else:
            self.alarm_label.config(text="✓ 正常", foreground="green")

        # LED 状态
        if sensor_data.get("led_blink", 0):
            self.led_label.config(text="↯ 闪烁中", foreground="orange")
        else:
            self.led_label.config(text="● 常亮", foreground="green")
        
    def run(self):
        def update_loop():
            while True:
                self.update_display()
                time.sleep(0.5)
                
        update_thread = threading.Thread(target=update_loop)
        update_thread.daemon = True
        update_thread.start()
        
        self.root.mainloop()


if __name__ == "__main__":
    app = SensorMonitorGUI()
    app.run()