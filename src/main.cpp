#include <Arduino.h>
#include "celdas.h"
#include "termistores.h"
#include "WebServerManager.h"
#include "Config.h"
#include "DisplayManager.h"

WebServerManager webServer;

// --- PINES ---
#define BOTON_ACCION_PIN 35
#define BOTON_TARA_PIN   39

// --- TIEMPOS ---
const unsigned long UPDATE_INTERVAL_MS = 100;
unsigned long lastUpdate = 0;

const unsigned long OLED_ROTATE_MS = 3000;
unsigned long lastOledUpdate = 0;

const unsigned long PAUSE_DURATION_MS = 5000;
unsigned long pauseEndTime = 0;

// --- NUEVO: PANTALLA GUARDAR DATOS ---
const unsigned long LISTA_TIMEOUT_MS = 30000;   
const unsigned long GUARDAR_SCREEN_MS = 3000;   
unsigned long listaStartTime = 0;
unsigned long guardarScreenStart = 0;

bool mostrarGuardar = false;
bool guardarMostrada = false;

// --- ESTADOS ---
bool medicionActiva = true;
int currentScreenIndex = 0;

// --- UTILIDADES ---
float gramsToKpa(float grams) {
    float val = grams * 0.0419f;
    return (val < 0) ? 0 : val;
}

// --- DEBOUNCE SIMPLE ---
bool isPressedAndReleased(int pin) {
    if (digitalRead(pin) == LOW) {
        delay(50);
        if (digitalRead(pin) == LOW) {
            while (digitalRead(pin) == LOW) {
                delay(10);
            }
            return true;
        }
    }
    return false;
}

// ============================================================
// SETUP
// ============================================================
void setup() {
    Serial.begin(115200);
    Serial.println("=== Iniciando Sistema (Modo Polling) ===");

    iniciarPantalla();
    pantallaLogo();

    pinMode(BOTON_ACCION_PIN, INPUT);
    pinMode(BOTON_TARA_PIN, INPUT);

    delay(2000);
    pantallaCargando();

    webServer.begin(WIFI_SSID, WIFI_PASSWORD, WEB_SERVER_PORT);

    initCeldas();
    initTermistores();

    pantallaLista();
    listaStartTime = millis();
    guardarMostrada = false;
    mostrarGuardar = false;

    delay(1000);

    medicionActiva = true;
    lastOledUpdate = millis();
}

// ============================================================
// LOOP
// ============================================================
void loop() {

    // ============================================================
    // 1. FIN DE PAUSA DE 5s
    // ============================================================
    if (!medicionActiva && pauseEndTime > 0 && millis() >= pauseEndTime) {
        Serial.println(">>> Fin de Pausa. Reanudando.");
        medicionActiva = true;
        pauseEndTime = 0;
        lastOledUpdate = millis();
    }

    // ============================================================
    // 2. BOTÓN TARA
    // ============================================================
    if (isPressedAndReleased(BOTON_TARA_PIN)) {
        Serial.println(">>> BOTON TARA");

        pantallaCalibrando();
        doTare();

        pantallaLista();
        listaStartTime = millis();
        guardarMostrada = false;
        mostrarGuardar = false;

        medicionActiva = true;
        pauseEndTime = 0;
        currentScreenIndex = 0;
        lastOledUpdate = millis();
    }

    // ============================================================
    // 3. BOTÓN ACCIÓN (PAUSA)
    // ============================================================
    if (isPressedAndReleased(BOTON_ACCION_PIN)) {
        Serial.println(">>> BOTON ACCION - HOLD 5s");

        medicionActiva = false;
        pauseEndTime = millis() + PAUSE_DURATION_MS;
    }

    // ============================================================
    // 4. PANTALLA "GUARDAR DATOS" TRANSITORIA
    // ============================================================
    if (medicionActiva && !guardarMostrada) {
        if (millis() - listaStartTime >= LISTA_TIMEOUT_MS) {
            pantallaGuardarDatos();   // <-- TU pantalla
            guardarScreenStart = millis();
            mostrarGuardar = true;
            guardarMostrada = true;
            Serial.println(">>> Pantalla Guardar Datos");
        }
    }

    if (mostrarGuardar) {
        if (millis() - guardarScreenStart >= GUARDAR_SCREEN_MS) {
            mostrarGuardar = false;
            lastOledUpdate = millis();
            Serial.println(">>> Retomando medición");
        } else {
            return; // Mientras se muestra, no actualiza valores
        }
    }

    // ============================================================
    // 5. SI NO ESTÁ MIDIENDO, SALIR
    // ============================================================
    if (!medicionActiva) {
        return;
    }

    // ============================================================
    // 6. MEDICIÓN + ENVÍO + OLED
    // ============================================================
    if (millis() - lastUpdate >= UPDATE_INTERVAL_MS) {
        lastUpdate = millis();

        float cellValues[16];
        getCellValues(cellValues);

        float temps[10];
        leerTodasLasTemperaturas(temps);

        float P_M5_L     = gramsToKpa(cellValues[7]);
        float P_M1_L     = gramsToKpa(cellValues[5]);
        float P_M1_R     = gramsToKpa(cellValues[0]);
        float P_M5_R     = gramsToKpa(cellValues[2]);
        float P_Side_L   = gramsToKpa(cellValues[8]);
        float P_Side_R   = gramsToKpa(cellValues[3]);
        float P_Heel_L   = gramsToKpa(cellValues[9]);
        float P_Heel_R   = gramsToKpa(cellValues[4]);
        float P_Hallux_L = gramsToKpa(cellValues[6]);
        float P_Hallux_R = gramsToKpa(cellValues[1]);

        float leftPress[]  = {P_M5_L, P_M1_L, P_Side_L, P_Heel_L, P_Hallux_L};
        float rightPress[] = {P_M1_R, P_M5_R, P_Side_R, P_Heel_R, P_Hallux_R};
        float leftTemp[]   = {temps[9], temps[5], temps[2], temps[1], temps[7]};
        float rightTemp[]  = {temps[6], temps[8], temps[3], temps[0], temps[4]};

        webServer.broadcastSensorData(leftPress, rightPress, leftTemp, rightTemp);

        if (millis() - lastOledUpdate > OLED_ROTATE_MS) {
            lastOledUpdate = millis();

            switch (currentScreenIndex) {
                case 0: mostrarSensor("Hallux Izq", 34, 12, P_Hallux_L, temps[2]); break;
                case 1: mostrarSensor("Metatarso 1 Izq", 17, 12, P_M1_L, temps[1]); break;
                case 2: mostrarSensor("Metatarso 5 Izq", 17, 12, P_M5_L, temps[7]); break;
                case 3: mostrarSensor("Lateral Izq", 31, 12, P_Side_L, temps[5]); break;
                case 4: mostrarSensor("Talon Izq", 37, 12, P_Heel_L, temps[9]); break;
                case 5: mostrarSensor("Hallux Der", 34, 12, P_Hallux_R, temps[3]); break;
                case 6: mostrarSensor("Metatarso 1 Der", 17, 12, P_M1_R, temps[0]); break;
                case 7: mostrarSensor("Metatarso 5 Der", 17, 12, P_M5_R, temps[4]); break;
                case 8: mostrarSensor("Lateral Der", 31, 12, P_Side_R, temps[8]); break;
                case 9: mostrarSensor("Talon Der", 37, 12, P_Heel_R, temps[6]); break;
            }

            currentScreenIndex = (currentScreenIndex + 1) % 10;
        }
    }
}
