# YoloUNO ESP32-S3 IoT Project

## Project Overview
This project is a comprehensive edge IoT system running on the **Yolo UNO (ESP32-S3)** hardware, designed with a robust, industry-standard software architecture.

### Key Architectural Concepts
- **FreeRTOS & Zero-Global Variable Architecture**: Assures memory safety and thread compatibility. All global variables are entirely eliminated using `Semaphore`, `Mutex`, and `Queue` structures. `SystemHandles` are securely injected into FreeRTOS Tasks for data transmission.
- **Fail-Safe Dual Mode Network (AP + STA)**: Ensures the device can continuously broadcast its internal Access Point (for local configuration) while simultaneously maintaining a Station connection to the internet, providing self-healing network capabilities.
- **Embedded Web Server (SPIFFS)**: Natively hosts a fully dynamic Web UI (HTML/CSS/JS) located in the device's Flash Memory, facilitating real-time sensor monitoring and hardware control via asynchronous AJAX/fetch mechanisms without page reloads.
- **TinyML Integration**: Employs an Edge ML framework utilizing TensorFlow Lite for Microcontrollers specifically on the ESP32-S3 to achieve in-device anomaly detection.
- **CoreIoT Synchronization**: Bridges physical sensors and actuations to the cloud data structure via MQTT/HTTP over the CoreIoT platform.

## Main Features
- Temperature and humidity acquisition from the DHT20 sensor over the I2C bus, securely guarded by a dedicated Mutex lock. 
- Real-time display of system conditions, sensor diagnostics, and states on an LCD monitor.
- Centralized hardware governance supporting a status LED, automated fan control, and a multi-color NeoPixel strip.
- Modern Web UI available over the network (Access locally via `192.168.4.1` on the `ESP32 LOCAL` Wi-Fi, or via a dynamic router IP).
- Overriding physical interrupt functionality via the board's embedded BOOT button.

## Directory Layout
- `src/` & `include/`: Houses the C/C++ foundation firmware code, including RTOS Tasks, Web Server handler, ML logic, and Sensor Handlers.
- `data/`: Contains the Web Server assets (`index.html`, `style.css`, visual icons, and javascript logic). This specific folder must be flashed directly into the SPIFFS partition independently.
- `platformio.ini`: Defines build flags, library dependencies, platform versions, and environment configurations.

---

## Quick Start & PlatformIO Guide

This firmware is exclusively built and maintained utilizing [PlatformIO](https://platformio.org/). 

### 1. Installation
1. Install [Visual Studio Code](https://code.visualstudio.com/).
2. Install the **PlatformIO IDE** extension from the VS Code Extensions marketplace and let the core tools install themselves.

### 2. Flashing the Filesystem (SPIFFS)
The graphical web interface files reside in the `data/` folder and must be uploaded separately from the main code.
1. Open this project folder in PlatformIO.
2. Go to the **PlatformIO Project Tasks** panel (the ant icon) -> `yolo_uno` -> `Platform`.
3. Click **Build Filesystem Image** then **Upload Filesystem Image** to flash the web resources onto the board.

### 3. Uploading Code & Monitoring
1. Within the same **Project Tasks** panel, head to `yolo_uno` -> `General`.
2. Click **Build** to verify, and then **Upload** to compile and push the C++ source code to the board.
3. Open the **Serial Monitor** (plug icon on the bottom toolbar) to verify operations at a `115200` baud rate.