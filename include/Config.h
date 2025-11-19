#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================
// WiFi Configuration
// ============================================
const char* WIFI_SSID = "Galaxysofi";        // Replace with your WiFi SSID
const char* WIFI_PASSWORD = "sofitacal"; // Replace with your WiFi password
// const char* WIFI_SSID = "Personal-51E-2.4GHz";        // Replace with your WiFi SSID
// // const char* WIFI_PASSWORD = "00421852415"; // Replace with your WiFi password
// const char* WIFI_SSID = "iPhone de Mora";        // Replace with your WiFi SSID
// const char* WIFI_PASSWORD = "moramora"; // Replace with your WiFi password
// ============================================
// Server Configuration
// ============================================
const int WEB_SERVER_PORT = 80;      // HTTP server port
const int WEBSOCKET_PORT = 81;       // WebSocket port (not used with AsyncWebSocket, included for reference)

// ============================================
// WebSocket Configuration
// ============================================
const unsigned long WS_UPDATE_INTERVAL_MS = 500; // WebSocket update interval (matches DISPLAY_UPDATE_INTERVAL_MS)

#endif

