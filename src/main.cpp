// #include "celdas.h"
// #include "termistores.h"
// #include "WebServerManager.h"
// #include "Config.h"
// #include <Arduino.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>

// WebServerManager webServer;

// const unsigned long UPDATE_INTERVAL_MS = 500;
// unsigned long lastUpdate = 0;

// bool medicionActiva = false;

// // ---------------- BOTÓN ----------------
// #define BOTON_PIN 10
// volatile bool botonPresionado = false;
// unsigned long lastInterruptTime = 0;

// void IRAM_ATTR botonInterrupt() {
//     unsigned long ms = millis();
//     if (ms - lastInterruptTime > 300) {   // debounce
//         botonPresionado = true;
//         lastInterruptTime = ms;
//     }
// }

// void setup() {
//     Serial.begin(115200);
//     Serial.println("=== Sistema iniciado ===");

//     // --- WiFi + Webserver ---
//     webServer.begin(WIFI_SSID, WIFI_PASSWORD, WEB_SERVER_PORT);

//     // --- Celdas ---
//     initCeldas();
//     Serial.println("Celdas iniciadas correctamente.");

//     // --- Termistores ---
//     initTermistores();
//     Serial.println("Termistores iniciados correctamente.");

//     // --- Botón ---
//     pinMode(BOTON_PIN, INPUT_PULLUP);
//     attachInterrupt(digitalPinToInterrupt(BOTON_PIN), botonInterrupt, FALLING);
//     Serial.println("Botón listo. Presione para iniciar medición.");
// }

// void loop() {

//     // Si hubo un toque de botón → alternar estado
//     if (botonPresionado) {
//         botonPresionado = false;
//         medicionActiva = !medicionActiva;

//         if (medicionActiva)
//             Serial.println(">>> Medición ACTIVADA por botón.");
//         else
//             Serial.println(">>> Medición PAUSADA por botón.");
//     }

//     // Si está pausada → no hacer nada
//     if (!medicionActiva) {
//         return;
//     }

//     // Medición activa
//     if (millis() - lastUpdate >= UPDATE_INTERVAL_MS) {
//         lastUpdate = millis();

//         // ============ CELDAS DE CARGA ============
//         float cellValues[8];
//         getCellValues(cellValues);

//         // LEFT FOOT
//         float leftPress[4];
//         leftPress[0] = cellValues[0]; // met5 left
//         leftPress[1] = cellValues[1]; // met1 left
//         leftPress[2] = cellValues[4]; // side left
//         leftPress[3] = cellValues[6]; // heel left

//         // RIGHT FOOT
//         float rightPress[4];
//         rightPress[0] = cellValues[2]; // met1 right
//         rightPress[1] = cellValues[3]; // met5 right
//         rightPress[2] = cellValues[5]; // side right
//         rightPress[3] = cellValues[7]; // heel right



//         // ============ TEMPERATURAS ============
//         float temps[10];
//         leerTodasLasTemperaturas(temps);

//      // LEFT → Talón, Lateral, Hallux, M1, M5
//         float leftTemp[5] = { 
//             temps[0], // talon 
//             temps[1], // lateral
//             temps[2], // hallux
//             temps[3], // met1
//             temps[4]  // met5
//         };

//         // RIGHT → Talón, Lateral, Hallux, M1, M5
//         float rightTemp[5] = { 
//             temps[5], // talon 
//             temps[6], // lateral
//             temps[7], // hallux
//             temps[8], // met1
//             temps[9]  // met5
//         };

//         Serial.println("Enviando datos...");

//         // *** ESTA ES LA LLAMADA CORRECTA ***
//         webServer.broadcastSensorData(leftPress, rightPress, leftTemp, rightTemp);
//     }
// }

#include "celdas.h"
#include "termistores.h"
#include "WebServerManager.h"
#include "Config.h"
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

WebServerManager webServer;

const unsigned long UPDATE_INTERVAL_MS = 500;
unsigned long lastUpdate = 0;

bool medicionActiva = false;

// ---------------- BOTÓN ----------------
#define BOTON_PIN 10
volatile bool botonPresionado = false;
unsigned long lastInterruptTime = 0;

// ===== Conversión gramos → kilopascales =====
float gramsToKpa(float grams) {
    return grams * 0.0419f;
}

void IRAM_ATTR botonInterrupt() {
    unsigned long ms = millis();
    if (ms - lastInterruptTime > 300) {   // debounce
        botonPresionado = true;
        lastInterruptTime = ms;
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("=== Sistema iniciado ===");

    // --- WiFi + Webserver ---
    webServer.begin(WIFI_SSID, WIFI_PASSWORD, WEB_SERVER_PORT);

    // --- Celdas ---
    initCeldas();
    Serial.println("Celdas iniciadas correctamente.");

    // --- Termistores ---
    initTermistores();
    Serial.println("Termistores iniciados correctamente.");

    // --- Botón ---
    pinMode(BOTON_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(BOTON_PIN), botonInterrupt, FALLING);
    Serial.println("Botón listo. Presione para iniciar medición.");
}

void loop() {

    // Si hubo un toque de botón → alternar estado
    if (botonPresionado) {
        botonPresionado = false;
        medicionActiva = !medicionActiva;

        if (medicionActiva)
            Serial.println(">>> Medición ACTIVADA por botón.");
        else
            Serial.println(">>> Medición PAUSADA por botón.");
    }

    // Si está pausada → no hacer nada
    if (!medicionActiva) {
        return;
    }

    // Medición activa
    if (millis() - lastUpdate >= UPDATE_INTERVAL_MS) {
        lastUpdate = millis();

        // ============ CELDAS DE CARGA ============
        float cellValues[8];
        getCellValues(cellValues);

        // LEFT FOOT
        float leftPress[4];
        leftPress[0] = gramsToKpa(cellValues[0]); // met5 left
        leftPress[1] = gramsToKpa(cellValues[1]); // met1 left
        leftPress[2] = gramsToKpa(cellValues[4]); // side left
        leftPress[3] = gramsToKpa(cellValues[6]); // heel left

        // RIGHT FOOT
        float rightPress[4];
        rightPress[0] = gramsToKpa(cellValues[2]); // met1 right
        rightPress[1] = gramsToKpa(cellValues[3]); // met5 right
        rightPress[2] = gramsToKpa(cellValues[5]); // side right
        rightPress[3] = gramsToKpa(cellValues[7]); // heel right

        // ============ TEMPERATURAS ============
        float temps[10];
        leerTodasLasTemperaturas(temps);

        // LEFT → Talón, Lateral, Hallux, M1, M5
        float leftTemp[5] = { 
            temps[0], // talon 
            temps[1], // lateral
            temps[2], // hallux
            temps[3], // met1
            temps[4]  // met5
        };

        // RIGHT → Talón, Lateral, Hallux, M1, M5
        float rightTemp[5] = { 
            temps[5], // talon 
            temps[6], // lateral
            temps[7], // hallux
            temps[8], // met1
            temps[9]  // met5
        };

        Serial.println("Enviando datos...");

        // ===== DEBUG: Mostrar conversión =====
        Serial.println("---- Valores en kPa ----");
        Serial.print("Left:  ");
        Serial.print(leftPress[0]); Serial.print("  ");
        Serial.print(leftPress[1]); Serial.print("  ");
        Serial.print(leftPress[2]); Serial.print("  ");
        Serial.println(leftPress[3]);

        Serial.print("Right: ");
        Serial.print(rightPress[0]); Serial.print("  ");
        Serial.print(rightPress[1]); Serial.print("  ");
        Serial.print(rightPress[2]); Serial.print("  ");
        Serial.println(rightPress[3]);


        // *** LLAMADA IGUAL QUE ANTES ***
        webServer.broadcastSensorData(leftPress, rightPress, leftTemp, rightTemp);
    }
}
