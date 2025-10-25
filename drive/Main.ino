#include "termistores.h"
#include "celdas.h" 
#include "WebServerManager.h"
#include "Config.h"
#include <Arduino.h>
#include <cmath> // Para NAN

// Instancias de clases
Termistores termistores; 
Celdas celdas;
WebServerManager webServer;           

// Definiciones de control de flujo
const unsigned long DISPLAY_UPDATE_INTERVAL_MS = 500; // Refresco de pantalla/web
unsigned long previousDisplayMillis = 0;


void setup() {
    // 1. Initialize WebServer and WiFi
    webServer.begin(WIFI_SSID, WIFI_PASSWORD, WEB_SERVER_PORT);

    // 2. Inicializar termistores (I2C)
    termistores.begin(); 

    // 3. Inicializar celdas (HX711)
    celdas.begin();
    
    // --- LÓGICA DE FEEDBACK DE INICIO (OLED/WEB) ---
    // El feedback inicial debe verificar AMBOS errores.
    if (termistores.initError() || celdas.initError()) {
        // Muestra error consolidado o específico
        String errorMsg = "";
        if (termistores.initError()) errorMsg += "ADS1115 ";
        if (celdas.initError()) errorMsg += "HX711 ";
        Serial.println("ERROR: " + errorMsg + " NO DETECTADO");
        // oled_display.showError("ERROR: " + errorMsg + " NO DETECTADO");
    } else {
        Serial.println("Sistema OK - Sensores iniciados correctamente");
        // oled_display.showMessage("Sistema OK");
    }
}

void loop() {
    // Tarea 1: Lectura de Termistores 
    // Lee un canal MUX por ciclo de loop (toma 10ms por lectura).
    termistores.update(); 
    
    // Tarea 2: Lectura de Celdas
    // Lee una celda por ciclo de loop (toma 50ms por lectura).
    celdas.update();
    
    // Tarea 3: Lógica de Visualización/Publicación de Datos (NO BLOQUEANTE)
    if (millis() - previousDisplayMillis >= DISPLAY_UPDATE_INTERVAL_MS) {
        previousDisplayMillis = millis();

        // Si hay un error, priorizar mostrar el error en pantalla/web
        if (termistores.initError() || celdas.initError()) {
            // (Re)mostrar mensaje de error si es necesario.
            // oled_display.forceShowError(); 
            return; // Salir del ciclo de publicación si hay error grave.
        }
        
        // --- PREPARAR DATOS PARA WEBSOCKET ---
        // Map sensor indices to left/right foot locations
        // Termistores: índices 0-4 = pie derecho, 5-9 = pie izquierdo
        // Celdas: índices 0-3 = pie derecho, 4-7 = pie izquierdo
        
        // Right foot (6 temperature sensors)
        float rightTemp[6];
        rightTemp[0] = termistores.getTempC(0); // heel
        rightTemp[1] = termistores.getTempC(1); // side
        rightTemp[2] = termistores.getTempC(2); // hallux
        rightTemp[3] = termistores.getTempC(3); // met1
        rightTemp[4] = termistores.getTempC(4); // met3
        rightTemp[5] = isnan(termistores.getTempC(4)) ? termistores.getTempC(4) : 0.0; // met5 (using met3 as placeholder since we only have 5)
        
        // Right foot (4 pressure sensors)
        float rightPress[4];
        rightPress[0] = celdas.getPressure(0); // heel
        rightPress[1] = celdas.getPressure(1); // met1
        rightPress[2] = celdas.getPressure(2); // met5
        rightPress[3] = celdas.getPressure(3); // mid
        
        // Left foot (6 temperature sensors)
        float leftTemp[6];
        leftTemp[0] = termistores.getTempC(5); // heel
        leftTemp[1] = termistores.getTempC(6); // side
        leftTemp[2] = termistores.getTempC(7); // hallux
        leftTemp[3] = termistores.getTempC(8); // met1
        leftTemp[4] = termistores.getTempC(9); // met3
        leftTemp[5] = isnan(termistores.getTempC(9)) ? termistores.getTempC(9) : 0.0; // met5 (using met3 as placeholder since we only have 10 total)
        
        // Left foot (4 pressure sensors)
        float leftPress[4];
        leftPress[0] = celdas.getPressure(4); // heel
        leftPress[1] = celdas.getPressure(5); // met1
        leftPress[2] = celdas.getPressure(6); // met5
        leftPress[3] = celdas.getPressure(7); // mid
        
        // Broadcast data via WebSocket
        webServer.broadcastSensorData(leftTemp, leftPress, rightTemp, rightPress);
            
        // Publicar Termistores (para debug serial si es necesario)
        /*
        for (int i = 0; i < N_TERMISTORES; i++) {
            float temp = termistores.getTempC(i);
            // displayData("T", i, temp);
        }

        // Publicar Celdas de Carga
        for (int i = 0; i < N_CELDAS; i++) {
            float pressure = celdas.getPressure(i);
            // displayData("P", i, pressure);
        }
        */

        // Una vez que todos los datos se han recopilado, actualizar la pantalla
        // oled_display.updateScreen();
    }
    
    // Tarea 4: Otras tareas continuas (Servidor Web, gestión de WiFi, etc.)
    // webServer.handleClient();
}