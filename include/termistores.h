#ifndef TERMISTORES_H
#define TERMISTORES_H

#include <Arduino.h>

void initTermistores();
float leerTemperaturaCanal(int canal);
void leerTodasLasTemperaturas(float outTemps[10]);

#endif
