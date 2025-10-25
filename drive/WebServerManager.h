#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

class WebServerManager {
public:
    WebServerManager();
    
    void begin(const char* ssid, const char* password, int port = 80);
    
    bool isConnected() const;
    
    String getIPAddress() const;
    
    void broadcastSensorData(
        const float leftTemp[6], 
        const float leftPress[4],
        const float rightTemp[6], 
        const float rightPress[4]
    );
    
    void handleWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                              AwsEventType type, void *arg, uint8_t *data, size_t len);

private:
    AsyncWebServer* server;
    AsyncWebSocket* ws;
    bool wifiConnected;
    
    bool initFileSystem();
    
    void setupRoutes();
    
    void connectWiFi(const char* ssid, const char* password);
};

#endif

