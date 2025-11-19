#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>

class WebServerManager {
public:
    WebServerManager();

    void begin(const char* ssid, const char* password, int port);
    bool isConnected() const;
    String getIPAddress() const;

    // =====================================================
    // 🚀 FUNCIÓN COMPLETA: presión + temperatura
    // =====================================================
    void broadcastSensorData(
        const float leftPress[4],
        const float rightPress[4],
        const float leftTemp[5],
        const float rightTemp[5]
    );

private:
    bool initFileSystem();
    void connectWiFi(const char* ssid, const char* password);
    void setupRoutes();

    AsyncWebServer* server;
    AsyncWebSocket* ws;
    bool wifiConnected;

    void handleWebSocketEvent(
        AsyncWebSocket *server, AsyncWebSocketClient *client,
        AwsEventType type, void *arg, uint8_t *data, size_t len
    );
};

#endif
