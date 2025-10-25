# Quick Start Guide - ESP32 WebSocket Sensor Monitor

## Step-by-Step Installation

### 1. Install Required Libraries

Open Arduino IDE Library Manager (Sketch > Include Library > Manage Libraries) and install:

- `Adafruit ADS1X15` by Adafruit
- `HX711 Arduino Library` by Bogdan Necula
- `ArduinoJson` by Benoit Blanchon (v6 or higher)

For `ESPAsyncWebServer` and `AsyncTCP` (not in Library Manager):

1. Download from GitHub:
   - ESPAsyncWebServer: https://github.com/me-no-dev/ESPAsyncWebServer/archive/master.zip
   - AsyncTCP: https://github.com/me-no-dev/AsyncTCP/archive/master.zip

2. In Arduino IDE: Sketch > Include Library > Add .ZIP Library
3. Select each downloaded ZIP file

### 2. Configure WiFi

Edit `drive/Config.h`:
```cpp
const char* WIFI_SSID = "YourWiFiName";
const char* WIFI_PASSWORD = "YourWiFiPassword";
```

### 3. Upload Filesystem (IMPORTANT!)

#### Option A: Using Arduino IDE Plugin

1. Download ESP32 LittleFS Plugin:
   - Go to: https://github.com/lorol/arduino-esp32littlefs-plugin/releases
   - Download the latest release ZIP
   - Extract to: `[Arduino_folder]/tools/ESP32LittleFS/tool/esp32littlefs.jar`
   - Folder structure should be: `Arduino/tools/ESP32LittleFS/tool/esp32littlefs.jar`

2. Restart Arduino IDE

3. Upload filesystem:
   - Tools > ESP32 LittleFS Data Upload
   - Wait for completion (may take 1-2 minutes)

#### Option B: Manual Method (if plugin doesn't work)

Use the **ESP32 Sketch Data Upload** tool with SPIFFS instead:
1. Download: https://github.com/me-no-dev/arduino-esp32fs-plugin/releases
2. Install to `Arduino/tools/`
3. Use: Tools > ESP32 Sketch Data Upload

### 4. Upload Sketch

1. Open `drive/Main.ino`
2. Select board: Tools > Board > ESP32 Arduino > ESP32 Dev Module
3. Select correct COM port: Tools > Port
4. Click Upload button
5. Wait for compilation and upload

### 5. Monitor Serial Output

1. Open Serial Monitor (Tools > Serial Monitor)
2. Set baud rate to: `115200`
3. You should see:
   ```
   === WebServer Manager Initialization ===
   LittleFS mounted successfully
   Connecting to WiFi: YourWiFiName
   ......
   WiFi connected!
   IP Address: 192.168.1.XXX
   === WebServer started successfully ===
   Access dashboard at: http://192.168.1.XXX
   ```

### 6. Access Dashboard

1. Note the IP address from Serial Monitor
2. Open browser on any device (phone, tablet, computer)
3. Navigate to: `http://192.168.1.XXX`
4. You should see the sensor dashboard with real-time data!

## Troubleshooting

### "LittleFS Mount Failed"
- You forgot to upload the filesystem! Go back to Step 3.

### "WiFi connection failed"
- Check SSID and password in Config.h
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)

### "404 Not Found" on web page
- Filesystem wasn't uploaded correctly
- Try uploading filesystem again (Step 3)

### Sensors showing "NAN" or error
- Check hardware connections
- Verify I2C and HX711 wiring
- See README.md for detailed pin connections

### WebSocket won't connect
- Check browser console (F12) for errors
- Ensure device and ESP32 are on same network
- Try clicking the refresh button

## Board Settings (Arduino IDE)

```
Board: "ESP32 Dev Module"
Upload Speed: "921600"
CPU Frequency: "240MHz"
Flash Frequency: "80MHz"
Flash Mode: "QIO"
Flash Size: "4MB (32Mb)"
Partition Scheme: "Default 4MB with spiffs"
Core Debug Level: "None"
```

## Next Steps

Once working:
- Adjust sensor mapping in Main.ino if needed
- Calibrate thermistors in termistores.cpp
- Calibrate load cells in celdas.cpp
- Customize web interface colors/layout in styles.css

## Need Help?

Check the detailed README.md in the same folder for:
- Complete hardware setup
- Sensor mapping details
- Calibration procedures
- Performance optimization

Good luck! 🚀

