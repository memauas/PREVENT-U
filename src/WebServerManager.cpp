#include "WebServerManager.h"

extern bool medicionActiva;

WebServerManager* wsManagerInstance = nullptr;

WebServerManager::WebServerManager() : wifiConnected(false) {
    server = nullptr;  
    ws = new AsyncWebSocket("/ws");
    wsManagerInstance = this;
}

void WebServerManager::begin(const char* ssid, const char* password, int port) {
    Serial.println("\n\n=== WebServer Manager Initialization ===");

    if (!initFileSystem()) return;

    connectWiFi(ssid, password);
    if (!wifiConnected) return;

    server = new AsyncWebServer(port);

    ws->onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client,
                   AwsEventType type, void *arg, uint8_t *data, size_t len) {
        if (wsManagerInstance) {
            wsManagerInstance->handleWebSocketEvent(server, client, type, arg, data, len);
        }
    });

    server->addHandler(ws);
    setupRoutes();
    server->begin();

    Serial.println("=== WebServer started successfully ===");
    Serial.print("Access dashboard at: http://");
    Serial.println(WiFi.localIP());
}

bool WebServerManager::initFileSystem() {
    if (!LittleFS.begin(true)) {
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
    } else {
        wifiConnected = false;
        Serial.println("\nWiFi connection failed!");
    }
}

void WebServerManager::setupRoutes() {
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });

    server->on("/styles.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/styles.css", "text/css");
    });

    server->on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/script.js", "text/javascript");
    });

    server->serveStatic("/images", LittleFS, "/images/");

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

void WebServerManager::handleWebSocketEvent(
    AsyncWebSocket *server, AsyncWebSocketClient *client,
    AwsEventType type, void *arg, uint8_t *data, size_t len) {

    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected\n", client->id());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        default:
            break;
    }
}

// =====================================================
// 🚀 Envía presión + temperatura (orden correctísimo)
// =====================================================
void WebServerManager::broadcastSensorData(
    const float leftPress[4],
    const float rightPress[4],
    const float leftTemp[5],
    const float rightTemp[5]
) {
    if (!medicionActiva) return;

    StaticJsonDocument<1024> doc;

    // -------- LEFT FOOT --------
    JsonObject left = doc.createNestedObject("left");

    JsonObject lt = left.createNestedObject("temperature");
    lt["heel"]    = leftTemp[0];
    lt["side"] = leftTemp[1];
    lt["hallux"]  = leftTemp[2];
    lt["met1"]    = leftTemp[3];
    lt["met5"]    = leftTemp[4];

    JsonObject lp = left.createNestedObject("pressure");
    lp["met5"] = leftPress[0];
    lp["met1"] = leftPress[1];
    lp["side"] = leftPress[2];
    lp["heel"] = leftPress[3];
    // -------- RIGHT FOOT --------
    JsonObject right = doc.createNestedObject("right");

    JsonObject rt = right.createNestedObject("temperature");
    rt["heel"]    = rightTemp[0];
    rt["side"] = rightTemp[1];
    rt["hallux"]  = rightTemp[2];
    rt["met1"]    = rightTemp[3];
    rt["met5"]    = rightTemp[4];

    JsonObject rp = right.createNestedObject("pressure");
    rp["met1"] = rightPress[0];
    rp["met5"] = rightPress[1];
    rp["side"] = rightPress[2];
    rp["heel"] = rightPress[3];

    String jsonString;
    serializeJson(doc, jsonString);
    ws->textAll(jsonString);
}
