#ifndef CELDAS_H
#define CELDAS_H

#include <Arduino.h>
#include "HX711.h" 

#define N_CELDAS 8

// Pines para las celdas (DT) y SCK compartido
const int PINS_DT[N_CELDAS] = {5, 17, 16, 4, 15, 14, 12, 13}; 
const int PIN_SCK = 18;

// Tiempo de espera para la lectura de la HX711 (puede ser 0, pero se usa para manejar latencia)
// La lectura de la HX711 es bloqueante si está lista, pero usamos el flag para llamarla solo a intervalos.
#define CELL_READ_INTERVAL_MS 50 // Intervalo mínimo entre la lectura de celdas.

class Celdas {
public:
    Celdas();

    void begin();
    void update();                             // Reemplaza a readAll(). Se llama constantemente.
    float getPressure(int index) const;        // devuelve presión en kPa
    bool initError() const;                    // Devuelve el estado del error

private:
    HX711 balanzas[N_CELDAS];
    float pressures[N_CELDAS] = {NAN};         // última presión leída
    float offsets[N_CELDAS] = {10.0, 12.5, 11.0, 9.5, 13.0, 10.5, 12.0, 11.5}; // Valores iniciales HAY QUE CAMBIAR!
    float scales[N_CELDAS]  = {2280.0, 2300.0, 2275.0, 2290.0, 2310.0, 2285.0, 2295.0, 2270.0}; // Valores iniciales HAY QUE CAMBIAR!
    float area_cm2 = 1.0;                      // área asociada a cada celda (modificable) HAY QUE CAMBIAR !!!!
    bool errorInit = false;                    // flag de error de inicialización

    // Variables para la implementación no bloqueante (millis)
    unsigned long previousCellReadTime = 0;    // Tiempo de la última lectura
    int currentCellIndex = 0;                  // Índice de la celda que se va a leer
    
    // Función auxiliar de cálculo
    float calculatePressure(float weight_g) const;
};

#endif