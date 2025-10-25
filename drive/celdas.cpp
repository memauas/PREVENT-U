#include "celdas.h"
#include <math.h> // Para NAN
#include <Arduino.h> // Para millis()


Celdas::Celdas() {}

/**
 * @brief Inicializa las celdas de carga HX711
 */
void Celdas::begin() {
    errorInit = false;

    for (int i = 0; i < N_CELDAS; i++) {
        // Inicializar la balanza con sus respectivos pines DT y SCK
        balanzas[i].begin(PINS_DT[i], PIN_SCK);
        // La librería HX711 requiere un pequeño tiempo después de begin() o power_up()
        // para estar completamente lista, aunque su lectura es por polling.
        // Hacemos un breve power_up/tare para forzar la respuesta inicial
        if (balanzas[i].is_ready()) {
            balanzas[i].power_up();
            // Aplicar offsets y escalas de calibración
            balanzas[i].set_offset(offsets[i]); 
            balanzas[i].set_scale(scales[i]); 
        } else {
            // Si la celda no responde inicialmente, marcamos el error
            errorInit = true; 
        }
    }
    // Inicializa el tiempo para la primera lectura no bloqueante
    previousCellReadTime = millis();
}

/**
 * @brief Implementa la conversión de peso (g) a presión (kPa)
 * @param weight_g Peso en gramos
 * @return Presión en kPa
 */
float Celdas::calculatePressure(float weight_g) const {
    // Fórmulas de conversión:
    // 1. Fuerza (N): F = peso [g] * 9.80665f / 1000.0f
    // 2. Área (m²): area_cm2 / 10000.0f (1 m² = 10000 cm²)
    // 3. Presión (Pa): P = F / Área
    // 4. Presión (kPa): P / 1000.0f
    
    // Simplificado a una sola operación:
    // P_kPa = (weight_g * 9.80665f) / (area_cm2 * 10.0f)

    if (area_cm2 <= 0.0) return NAN;
    return (weight_g * 9.80665f / (area_cm2 * 10000.0f)) / 1000.0f; // Pa -> kPa
}


/**
 * @brief Actualiza la presión de una sola celda de forma no bloqueante (máquina de estados)
 * * Se llama constantemente en loop().
 */
void Celdas::update() {
    // Solo si no hay error de inicialización
    if (errorInit) return;

    // Solo leer si ha pasado el intervalo de tiempo desde la última lectura
    if (millis() - previousCellReadTime >= CELL_READ_INTERVAL_MS) {
        
        // 1. LEER LA CELDA ACTUAL
        int i = currentCellIndex;

        if (balanzas[i].is_ready()) {
            // NOTA: La función get_units(1) es semi-bloqueante si no hay datos.
            // Para ser 100% no bloqueante, se debería usar read() o check_ready().
            // Asumimos que la latencia de get_units(1) es aceptable dado el intervalo de 50ms.
            float weight_g = balanzas[i].get_units(1); // lectura en gramos
            pressures[i] = calculatePressure(weight_g); 
        } else {
            // Si no está lista, dejamos NAN, pero no marcamos error de init a menos que sea persistente.
            pressures[i] = NAN; 
        }

        // 2. PREPARAR PARA LA PRÓXIMA CELDA
        currentCellIndex++;
        if (currentCellIndex >= N_CELDAS) {
            currentCellIndex = 0; // Volver al inicio
        }

        // Actualizar el tiempo del último cambio
        previousCellReadTime = millis();
    }
}

/**
 * @brief Devuelve la presión medida
 */
float Celdas::getPressure(int index) const {
    if (index < 0 || index >= N_CELDAS) return NAN;
    return pressures[index];
}

/**
 * @brief Devuelve el estado de error de inicialización
 */
bool Celdas::initError() const {
    return errorInit;
}