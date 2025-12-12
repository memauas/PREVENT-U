// /**
//  * main.cpp
//  * Integración Final: Celdas + Termistores + WebServer + OLED U8g2
//  * Control: GPIO 35 (Pausa/Inicio), GPIO 39 (Tara)
//  */

// #include <Arduino.h>
// #include "celdas.h"
// #include "termistores.h"
// #include "WebServerManager.h"
// #include "Config.h"
// #include "DisplayManager.h"

// WebServerManager webServer;

// // ---------------- DEFINICIÓN DE PINES DE BOTONES ----------------
// // Ambos son INPUT ONLY en ESP32 (Sin Pull-up interno)
// // Requieren resistencia externa.
// #define BOTON_ACCION_PIN 35   // Inicio / Pausa
// #define BOTON_TARA_PIN   39   // Tara (Poner a cero)

// // ---------------- VARIABLES DE TIEMPO ----------------
// const unsigned long UPDATE_INTERVAL_MS = 100; // Lectura rápida de sensores
// unsigned long lastUpdate = 0;

// const unsigned long OLED_ROTATE_MS = 3000;    // Tiempo por pantalla
// unsigned long lastOledUpdate = 0;

// // ---------------- ESTADOS DEL SISTEMA ----------------
// bool medicionActiva = false;  // false = PAUSADO, true = MIDIENDO
// int currentScreenIndex = 0;   // Índice de la pantalla rotativa (0-9)

// // ---------------- INTERRUPCIONES (Banderas) ----------------
// volatile bool accionPresionada = false; // Bandera para botón 35
// volatile bool taraSolicitada = false;   // Bandera para botón 39
// unsigned long lastInterruptTime = 0;

// // ISR: Interrupción Botón Acción (GPIO 35)
// void IRAM_ATTR isrBotonAccion() {
//     unsigned long ms = millis();
//     if (ms - lastInterruptTime > 300) { // Debounce de 300ms
//         accionPresionada = true;
//         lastInterruptTime = ms;
//     }
// }

// // ISR: Interrupción Botón Tara (GPIO 39)
// void IRAM_ATTR isrBotonTara() {
//     unsigned long ms = millis();
//     if (ms - lastInterruptTime > 300) { 
//         taraSolicitada = true;
//         lastInterruptTime = ms;
//     }
// }

// // ---------------- UTILIDADES ----------------
// float gramsToKpa(float grams) {
//     float val = grams * 0.0419f;
//     return (val < 0) ? 0 : val;
// }

// // ---------------- SETUP ----------------
// void setup() {
//     Serial.begin(115200);
//     Serial.println("=== Sistema Iniciando ===");

//     // 1. Iniciar Pantalla
//     iniciarPantalla();
//     pantallaLogo();

//     // 2. Configurar Botones (INPUT normal porque son pines 34-39)
//     pinMode(BOTON_ACCION_PIN, INPUT); 
//     pinMode(BOTON_TARA_PIN, INPUT);

//     // Configurar interrupciones
//     // FALLING significa: Se activa cuando el voltaje cae (Botón conecta a GND)
//     // Si tus botones conectan a 3.3V, cambia FALLING por RISING
//     attachInterrupt(digitalPinToInterrupt(BOTON_ACCION_PIN), isrBotonAccion, FALLING);
//     attachInterrupt(digitalPinToInterrupt(BOTON_TARA_PIN), isrBotonTara, FALLING);

//     delay(2000); // Ver logo
//     pantallaCargando();

//     // 3. Iniciar Servicios y Sensores
//     webServer.begin(WIFI_SSID, WIFI_PASSWORD, WEB_SERVER_PORT);
    
//     // Inicializa celdas (incluye tara inicial automática)
//     initCeldas();     
//     initTermistores();
    
//     pantallaLista(); // Muestra "Plataforma lista" y espera al botón 35
//     delay(1000);
// }

// // ---------------- LOOP ----------------
// void loop() {

//     // A. VERIFICAR TARA (Botón 39)
//     if (taraSolicitada) {
//         taraSolicitada = false;
//         Serial.println(">>> TARA SOLICITADA");
        
//         pantallaCalibrando(); // Muestra aviso visual
//         doTare();             // Función bloqueante (unos segundos)
        
//         // Restaurar pantalla anterior
//         if (medicionActiva) lastOledUpdate = 0; // Forzar refresh inmediato
//         else pantallaLista(); 
//     }

//     // B. VERIFICAR INICIO/PAUSA (Botón 35)
//     if (accionPresionada) {
//         accionPresionada = false;
//         medicionActiva = !medicionActiva; // Alternar estado
        
//         // Resetear temporizadores visuales
//         currentScreenIndex = 0; 
//         lastOledUpdate = millis();

//         if (medicionActiva) {
//             Serial.println(">>> MEDICION INICIADA");
//         } else {
//             Serial.println(">>> PAUSA");
//             mostrarPausa(webServer.getIPAddress());
//         }
//     }

//     // C. LÓGICA DE PAUSA
//     if (!medicionActiva) {
//         // Refrescamos la pantalla de pausa cada segundo por si acaso
//         if (millis() - lastOledUpdate > 1000) {
//             if(!taraSolicitada) mostrarPausa(webServer.getIPAddress());
//             lastOledUpdate = millis();
//         }
//         return; // <--- AQUÍ CORTAMOS EL LOOP SI ESTÁ PAUSADO
//     }

//     // D. LÓGICA DE MEDICIÓN (Solo si medicionActiva == true)
//     if (millis() - lastUpdate >= UPDATE_INTERVAL_MS) {
//         lastUpdate = millis();

//         // 1. Leer Sensores
//         float cellValues[10]; 
//         getCellValues(cellValues); 
        
//         float temps[10];
//         leerTodasLasTemperaturas(temps);

//         // 2. Procesar Datos (Mapeo)
//         // Ajusta los índices [0..7] según tu cableado real
//         float P_M5_L = gramsToKpa(cellValues[7]); 
//         float P_M1_L = gramsToKpa(cellValues[5]); 
//         float P_M1_R = gramsToKpa(cellValues[0]); 
//         float P_M5_R = gramsToKpa(cellValues[2]);
//         float P_Side_L = gramsToKpa(cellValues[8]);
//         float P_Side_R = gramsToKpa(cellValues[3]);
//         float P_Heel_L = gramsToKpa(cellValues[9]);
//         float P_Heel_R = gramsToKpa(cellValues[4]);
//         float P_Hallux_L = gramsToKpa(cellValues[6]);
//         float P_Hallux_R = gramsToKpa(cellValues[1]); 
        
//         // 3. Enviar a WebServer
//         float leftPress[] = { P_M5_L, P_M1_L, P_Side_L, P_Heel_L, P_Hallux_L};
//         float rightPress[] = { P_M1_R, P_M5_R, P_Side_R, P_Heel_R, P_Hallux_R};
//         float leftTemp[] = { temps[9], temps[5], temps[2], temps[1], temps[7] };
//         float rightTemp[] = { temps[6], temps[8], temps[3], temps[0], temps[4] };

//         webServer.broadcastSensorData(leftPress, rightPress, leftTemp, rightTemp);

//         // 4. Actualizar Pantalla OLED (Rotación automática)
//         if (millis() - lastOledUpdate > OLED_ROTATE_MS) {
//             lastOledUpdate = millis();
            
//             // Ciclo de pantallas
//             switch (currentScreenIndex) {
//                 // PIE IZQUIERDO
//                 case 0: mostrarSensor("Hallux Izq", 16, 12, P_Hallux_L, temps[2]); break;
//                 case 1: mostrarSensor("Met 1 Izq", 16, 12, P_M1_L, temps[1]); break;
//                 case 2: mostrarSensor("Met 5 Izq", 16, 12, P_M5_L, temps[7]); break;
//                 case 3: mostrarSensor("Lateral Izq", 16, 12, P_Side_L, temps[5]); break;
//                 case 4: mostrarSensor("Talon Izq", 16, 12, P_Heel_L, temps[9]); break;

//                 // PIE DERECHO
//                 case 5: mostrarSensor("Hallux Der", 22, 12, P_Hallux_R, temps[3]); break;
//                 case 6: mostrarSensor("Met 1 Der", 22, 12, P_M1_R, temps[0]); break;
//                 case 7: mostrarSensor("Met 5 Der", 22, 12, P_M5_R, temps[4]); break;
//                 case 8: mostrarSensor("Lateral Der", 22, 12, P_Side_R, temps[8]); break;
//                 case 9: mostrarSensor("Talon Der", 22, 12, P_Heel_R, temps[6]); break;
//             }

//             // Siguiente pantalla
//             currentScreenIndex++;
//             if (currentScreenIndex > 9) currentScreenIndex = 0;
//         }
//     }
// }

/**
 * main.cpp
 * SOLUCIÓN FINAL: Polling (Lectura directa) en lugar de Interrupciones
 * Hardware: Resistencias Pull-up externas en pines 35 y 39.
 */

#include <Arduino.h>
#include "celdas.h"     
#include "termistores.h"
#include "WebServerManager.h"
#include "Config.h"
#include "DisplayManager.h"

WebServerManager webServer;

// --- PINES (Con Resistencia Externa Pull-up) ---
#define BOTON_ACCION_PIN 35   
#define BOTON_TARA_PIN   39   

// --- TIEMPOS ---
const unsigned long UPDATE_INTERVAL_MS = 100;
unsigned long lastUpdate = 0;

const unsigned long OLED_ROTATE_MS = 3000;
unsigned long lastOledUpdate = 0;

// --- ESTADOS ---
bool medicionActiva = false;
int currentScreenIndex = 0;

// --- UTILIDADES ---
float gramsToKpa(float grams) {
    float val = grams * 0.0419f;
    return (val < 0) ? 0 : val;
}

void setup() {
    Serial.begin(115200);
    Serial.println("=== Iniciando Sistema (Modo Polling) ===");

    iniciarPantalla();
    pantallaLogo();

    // Configuración INPUT (Igual que en tu código de prueba)
    pinMode(BOTON_ACCION_PIN, INPUT); 
    pinMode(BOTON_TARA_PIN, INPUT);
    
    // NOTA: YA NO USAMOS attachInterrupt() PORQUE CAUSA RUIDO

    delay(2000);
    pantallaCargando();

    webServer.begin(WIFI_SSID, WIFI_PASSWORD, WEB_SERVER_PORT);
    
    initCeldas();     
    initTermistores();
    
    pantallaLista(); 
    delay(1000);
}

void loop() {
    
    // ============================================================
    // 1. LECTURA DE BOTÓN TARA (GPIO 39)
    // ============================================================
    // Si leemos LOW (0), es que se está apretando
    if (digitalRead(BOTON_TARA_PIN) == LOW) {
        delay(50); // Pequeña espera para asegurar que no es ruido (Debounce)
        
        if (digitalRead(BOTON_TARA_PIN) == LOW) {
            // Confirmado: Se apretó el botón
            Serial.println(">>> BOTON TARA DETECTADO");
            
            // Esperamos a que sueltes el botón para no disparar 2 veces
            while(digitalRead(BOTON_TARA_PIN) == LOW) { delay(10); }

            // --- EJECUTAR ACCIÓN ---
            pantallaCalibrando();
            doTare(); 
            
            // Volver a la pantalla correcta
            if (medicionActiva) lastOledUpdate = 0; 
            else pantallaLista(); 
        }
    }

    // ============================================================
    // 2. LECTURA DE BOTÓN INICIO/PAUSA (GPIO 35)
    // ============================================================
    if (digitalRead(BOTON_ACCION_PIN) == LOW) {
        delay(50); // Debounce
        
        if (digitalRead(BOTON_ACCION_PIN) == LOW) {
            // Confirmado
            Serial.println(">>> BOTON ACCION DETECTADO");
            
            // Esperamos a que sueltes
            while(digitalRead(BOTON_ACCION_PIN) == LOW) { delay(10); }

            // --- EJECUTAR ACCIÓN ---
            medicionActiva = !medicionActiva; // Cambiar estado
            currentScreenIndex = 0; 
            lastOledUpdate = millis();

            if (medicionActiva) {
                Serial.println(">>> ESTADO: MIDIENDO");
            } else {
                Serial.println(">>> ESTADO: PAUSA");
                mostrarPausa(webServer.getIPAddress());
            }
        }
    }

    // ============================================================
    // 3. LOGICA DEL SISTEMA
    // ============================================================
    
    // Si está pausado, salimos aquí y volvemos al principio del loop
    // para seguir chequeando botones.
    if (!medicionActiva) {
        if (millis() - lastOledUpdate > 1000) {
            // Refrescar pantalla de pausa ocasionalmente
            mostrarPausa(webServer.getIPAddress());
            lastOledUpdate = millis();
        }
        return; 
    }

    // MEDICIÓN ACTIVA
    if (millis() - lastUpdate >= UPDATE_INTERVAL_MS) {
        lastUpdate = millis();

        // Array seguro de 16 posiciones para evitar el error de memoria anterior
        float cellValues[16]; 
        getCellValues(cellValues); 
        
        float temps[10];
        leerTodasLasTemperaturas(temps);

        // Mapeo seguro
        float P_M5_L = gramsToKpa(cellValues[7]); 
        float P_M1_L = gramsToKpa(cellValues[5]); 
        float P_M1_R = gramsToKpa(cellValues[0]); 
        float P_M5_R = gramsToKpa(cellValues[2]);
        float P_Side_L = gramsToKpa(cellValues[8]); 
        float P_Side_R = gramsToKpa(cellValues[3]);
        float P_Heel_L = gramsToKpa(cellValues[9]); 
        float P_Heel_R = gramsToKpa(cellValues[4]);
        float P_Hallux_L = gramsToKpa(cellValues[6]);
        float P_Hallux_R = gramsToKpa(cellValues[1]); 

        // Enviar Web
        float leftPress[] = { P_M5_L, P_M1_L, P_Side_L, P_Heel_L, P_Hallux_L};
        float rightPress[] = { P_M1_R, P_M5_R, P_Side_R, P_Heel_R, P_Hallux_R};
        float leftTemp[] = { temps[9], temps[5], temps[2], temps[1], temps[7] };
        float rightTemp[] = { temps[6], temps[8], temps[3], temps[0], temps[4] };

        webServer.broadcastSensorData(leftPress, rightPress, leftTemp, rightTemp);

        // OLED Rotativo
        if (millis() - lastOledUpdate > OLED_ROTATE_MS) {
            lastOledUpdate = millis();
            
            switch (currentScreenIndex) {
                case 0: mostrarSensor("Hallux Izq", 16, 12, P_Hallux_L, temps[2]); break;
                case 1: mostrarSensor("Met 1 Izq", 16, 12, P_M1_L, temps[1]); break;
                case 2: mostrarSensor("Met 5 Izq", 16, 12, P_M5_L, temps[7]); break;
                case 3: mostrarSensor("Lateral Izq", 16, 12, P_Side_L, temps[5]); break;
                case 4: mostrarSensor("Talon Izq", 16, 12, P_Heel_L, temps[9]); break;
                case 5: mostrarSensor("Hallux Der", 22, 12, P_Hallux_R, temps[3]); break;
                case 6: mostrarSensor("Met 1 Der", 22, 12, P_M1_R, temps[0]); break;
                case 7: mostrarSensor("Met 5 Der", 22, 12, P_M5_R, temps[4]); break;
                case 8: mostrarSensor("Lateral Der", 22, 12, P_Side_R, temps[8]); break;
                case 9: mostrarSensor("Talon Der", 22, 12, P_Heel_R, temps[6]); break;
            }
            currentScreenIndex++;
            if (currentScreenIndex > 9) currentScreenIndex = 0;
        }
    }
}