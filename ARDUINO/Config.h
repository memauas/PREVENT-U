#ifndef CONFIG_H
#define CONFIG_H

// ============================================
// WiFi Configuration
// ============================================
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // Replace with your WiFi SSID
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // Replace with your WiFi password

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

