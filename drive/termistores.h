#ifndef TERMISTORES_H
#define TERMISTORES_H

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>

// --- CONSTANTES ---

#define N_TERMISTORES 10    // total de termistores

// Pines del multiplexor 
#define MUX_S0 13
#define MUX_S1 14
#define MUX_S2 27
#define MUX_S3 26

// Constante del ADS1115 (mV/bit con GAIN_ONE)
#define MV_PER_BIT 0.125 

// Tiempo de espera para la estabilización del MUX (en ms)
#define MUX_STABILIZATION_TIME_MS 10 

// --- ESTRUCTURAS ---

// Estructura de configuración para cada termistor
struct Termistor {
    int canalMux;         // canal en el MUX (0–15)
    float A, B, C;        // coeficientes Steinhart–Hart
    float linA;           // corrección lineal (pendiente)
    float linB;           // corrección lineal (offset)
};

class Termistores {
public:
    Termistores();
    void begin(uint8_t sda_pin = 21, uint8_t scl_pin = 22); // Añadir pines I2C opcionales
    void update();                             // Reemplaza a readAll(). Se llama constantemente.
    bool initError() const;                    // const: no modifica el estado del objeto
    float getTempC(int index) const;           // const: no modifica el estado del objeto

private:
    Adafruit_ADS1115 ads; 
    float tempsC[N_TERMISTORES] = {0.0};       // Inicializado a 0.0
    bool errorFlag = false;                    // Inicializado a false

    // Variables para la implementación no bloqueante (millis)
    unsigned long previousMuxSwitchTime = 0;   // Tiempo del último cambio de canal
    int currentTermistorIndex = 0;             // Índice del termistor que se va a leer
    
    // Funciones auxiliares
    void seleccionarCanal(int canal); 
    float calculateTempC(int16_t rawADC, const Termistor &t); // Función de cálculo
};


#endif
