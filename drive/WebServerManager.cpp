#include "WebServerManager.h"

WebServerManager* wsManagerInstance = nullptr;

WebServerManager::WebServerManager() : wifiConnected(false) {
    server = new AsyncWebServer(80);
    ws = new AsyncWebSocket("/ws");
    wsManagerInstance = this;
}

void WebServerManager::begin(const char* ssid, const char* password, int port) {
    Serial.begin(115200);
    Serial.println("\n\n=== WebServer Manager Initialization ===");
    
    // 1. Initialize filesystem
    if (!initFileSystem()) {
        Serial.println("ERROR: Failed to initialize LittleFS");
        return;
    }
    
    // 2. Connect to WiFi
    connectWiFi(ssid, password);
    
    if (!wifiConnected) {
        Serial.println("ERROR: WiFi connection failed");
        return;
    }
    
    // 3. Setup WebSocket
    ws->onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, 
                   AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (wsManagerInstance) {
            wsManagerInstance->handleWebSocketEvent(server, client, type, arg, data, len);
        }
    });
    server->addHandler(ws);
    
    // 4. Setup routes
    setupRoutes();
    
    // 5. Start server
    server->begin();
    Serial.println("=== WebServer started successfully ===");
    Serial.print("Access dashboard at: http://");
    Serial.println(WiFi.localIP());
}

bool WebServerManager::initFileSystem() {
    if (!LittleFS.begin(true)) { // true = format if mount fails
        Serial.println("LittleFS Mount Failed");
        return false;
    }
    Serial.println("LittleFS mounted successfully");
    return true;
}

void WebServerManager::connectWiFi(const char* ssid, const char* password) {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println("\nWiFi connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.print("Signal Strength: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");
    } else {
        wifiConnected = false;
        Serial.println("\nWiFi connection failed!");
    }
}

void WebServerManager::setupRoutes() {
    // Serve index.html at root
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });
    
    // Serve static files
    server->on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/styles.css", "text/css");
    });
    
    server->on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/script.js", "text/javascript");
    });
    
    server->on("/images/pie_izquierdo.png", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/images/pie_izquierdo.png", "image/png");
    });
    
    // 404 handler
    server->onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not found");
    });
}

bool WebServerManager::isConnected() const {
    return wifiConnected && (WiFi.status() == WL_CONNECTED);
}

String WebServerManager::getIPAddress() const {
    if (isConnected()) {
        return WiFi.localIP().toString();
    }
    return "Not Connected";
}

void WebServerManager::handleWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                                            AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            // Handle incoming WebSocket data if needed
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void WebServerManager::broadcastSensorData(
    const float leftTemp[6], 
    const float leftPress[4],
    const float rightTemp[6], 
    const float rightPress[4]
) {
    // Create JSON document (adjust size if needed)
    StaticJsonDocument<1024> doc;
    
    // Left foot data
    JsonObject left = doc.createNestedObject("left");
    JsonObject leftTemperature = left.createNestedObject("temperature");
    leftTemperature["heel"] = String(leftTemp[0], 1);
    leftTemperature["side"] = String(leftTemp[1], 1);
    leftTemperature["hallux"] = String(leftTemp[2], 1);
    leftTemperature["met1"] = String(leftTemp[3], 1);
    leftTemperature["met3"] = String(leftTemp[4], 1);
    leftTemperature["met5"] = String(leftTemp[5], 1);
    
    JsonObject leftPressure = left.createNestedObject("pressure");
    leftPressure["heel"] = String(leftPress[0], 1);
    leftPressure["met1"] = String(leftPress[1], 1);
    leftPressure["met5"] = String(leftPress[2], 1);
    leftPressure["mid"] = String(leftPress[3], 1);
    
    // Right foot data
    JsonObject right = doc.createNestedObject("right");
    JsonObject rightTemperature = right.createNestedObject("temperature");
    rightTemperature["heel"] = String(rightTemp[0], 1);
    rightTemperature["side"] = String(rightTemp[1], 1);
    rightTemperature["hallux"] = String(rightTemp[2], 1);
    rightTemperature["met1"] = String(rightTemp[3], 1);
    rightTemperature["met3"] = String(rightTemp[4], 1);
    rightTemperature["met5"] = String(rightTemp[5], 1);
    
    JsonObject rightPressure = right.createNestedObject("pressure");
    rightPressure["heel"] = String(rightPress[0], 1);
    rightPressure["met1"] = String(rightPress[1], 1);
    rightPressure["met5"] = String(rightPress[2], 1);
    rightPressure["mid"] = String(rightPress[3], 1);
    
    // Serialize and send
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Broadcast to all connected clients
    ws->textAll(jsonString);
}

