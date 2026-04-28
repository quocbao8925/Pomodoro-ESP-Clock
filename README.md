# 🍅 POMODORO ESPsmartCLOCK

A highly optimized, gyroscope smart clock and Pomodoro timer built with the **ESP32-C3 Mini** and **ESP-IDF**. This project focuses on deep system optimization, dynamic power management, and a modern Web Serial interface for configuration.

## Key Features

* **Gesture Control:** Uses a **BMI160** 6-axis IMU to navigate menus and start timers simply by flipping device.
* **Pomodoro Timer:** Built-in 25-minute focus and 5-minute break timers with automated buzzer notifications.
* **Internet Time & Weather:** Real-time synchronization via SNTP and live weather updates via the OpenWeatherMap API.
* **Web Serial Configuration:** Say goodbye to clunky WiFi AP captive portals! Configure WiFi credentials, buzzer volume, time formats, and screen timeouts directly via a secure USB Web Serial HTML interface.
* **Ultra-Low Power Optimization:**
  * **Adaptive Polling:** The IMU polling rate dynamically drops when the screen is off to conserve battery.
  * **NVS Wear-Leveling:** Device settings are safely stored in Non-Volatile Storage with optimized commit triggers.

## Hardware Requirements

* **Microcontroller:** ESP32-C3 Mini
* **Display:** 0.96" OLED Display
* **IMU Sensor:** BMI160
* **Audio:** Passive Buzzer

## Tech Stack & Libraries

* **Framework:** [ESP-IDF v5+](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/get-started/index.html)
* **Graphics:** [u8g2](https://github.com/olikraus/u8g2)
* **Networking:** `esp_wifi`, `esp_http_client`, `esp_sntp`

## Getting Started
### 1. Build and Flash
Clone this repository and build it using the ESP-IDF toolchain.

```bash
git clone https://github.com/quocbao8925/Pomodoro-ESP-Clock.git
cd Pomodoro-ESP-Clock
```
Set target, build and flash.
```bash
idf.py set-target esp32c3
idf.py build flash monitor
```
### 2. Device Configuration (Web Serial)
Instead of hardcoding your WiFi credentials, use the provided Web Interface:
1. Open `config-web` (located in the repo).
2. Connect the ESP32-C3 via USB.
3. Click **Connect Cable** and select your COM port.
4. On the watch, flip your device to navigate to the **CONFIG** screen.
5. Enter your WiFi SSID, Password, and adjust system settings (Volume, Timeout, 24h format) on the webpage.
6. Click **Sync Data & Settings**. The watch will automatically reboot and apply the changes.

## API Keys Setup
To enable weather updates, you need to configure your [OpenWeatherMap](https://openweathermap.org/api) credentials via the ESP-IDF menu:
1. Run `idf.py menuconfig`.
2. Navigate to **Project Configuration**.
3. Enter your Latitude, Longitude, and OpenWeatherMap API Key.
4. Save (S) and exit (Q).
   
## 🔗 Related Repository
This firmware is designed to work with the configuration web and u8g2 library. </br>
POMODORO Web's URL: [Link to web](https://quocbao8925.github.io/pomodoro/) </br>
Repo URL: [Link to Repo](https://github.com/quocbao8925/pomodoro) </br>
U8G2: [u8g2 library](https://github.com/olikraus/u8g2)

## Inspired By
This project was heavily inspired by an amazing work of HuyVectorDIY. </br>
Check out the original build here: [Cyber Smart Clock V2](https://www.huyvector.org/clocks/smart-cyber-clock-v2).

## License
This project is open-source and available under the [MIT License](https://www.google.com/search?q=LICENSE).
