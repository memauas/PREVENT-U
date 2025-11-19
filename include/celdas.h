#ifndef CELDAS_H
#define CELDAS_H

#include <Arduino.h>
#include "HX711.h"

// --- Cantidad de celdas ---
#define NUM_CELDAS 8

// Inicializa pines, HX711, MUX y hace tare inicial
void initCeldas();

// Re-tareo manual
void doTare();

// Devuelve las 8 celdas filtradas (EMA)
void getCellValues(float outValues[NUM_CELDAS]);

#endif
