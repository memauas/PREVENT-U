/**
 * DisplayManager.h
 * Declaraciones de funciones para la pantalla OLED
 */

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>

// Definición de Pines (Si usas el mismo de antes)
// Nota: En tu main usas el pin 10 para el botón, aquí lo dejamos solo como referencia gráfica si hace falta
// pero la lógica del botón la maneja tu main.cpp.

// Funciones
void iniciarPantalla();
void pantallaLogo();
void pantallaCargando();
void pantallaLista();

// Función principal de visualización
void mostrarSensor(const char* titulo, int x, int y, float presion, float temp);

// Función para mostrar estado de pausa con IP
void mostrarPausa(String ip);

// Función para tara
void pantallaCalibrando();

#endif