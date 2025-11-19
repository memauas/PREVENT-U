#include "termistores.h"
#include <Wire.h>
#include <math.h>

// --- CONFIGURACIÓN DE TERMISTORES ---
Termistor termistores[N_TERMISTORES] = {
    // Pie der 1
    {1, 0.0003445997280, 0.0003633891147, -4.4784468504e-7, 1.02, -0.4}, 
    {2, 0.0002314524890, 0.0003842840095, -5.4957972017e-7, 1.01, -0.3},
    {3, 0.0005570546276954971, 0.00033139033907615044, -3.4260831077600784e-7, 1.00, 0.0},
    {4, -0.00003326439257207974, 0.00042455159833172944, -6.796937591738187e-7, 1.00, 0.0},
    {5, 0.0005184773245019295, 0.0003382271771314894, -3.6886918108065415e-7, 1.00, 0.0},
    // Pie izq 1
    {6, 0.0005088505804312611, 0.00034258920559030414, -4.0480253295691954e-7, 1.00, 0.0}, 
    {7, 0.0006721010744610125, 0.0003083060621826364, -2.1460764589504245e-7, 1.00, 0.0},
    {8, 0.00036097231797998673, 0.0003621767940661376, -4.4098669538497415e-7, 1.00, 0.0},
    {9, 0.0006409455560852659, 0.0003150994053334256, -2.552694870031203e-7, 1.00, 0.0},
    {10, 0.0006081904899106184, 0.0003184497747441781, -2.436439401032618e-7, 1.00, 0.0} 

// Resistencias del divisor
const float R1 = 10000.0;    // serie
const float R2 = 10000.0;    // paralelo
const float VCC = 3.3;       // alimentación del divisor

// --- IMPLEMENTACIÓN ---

Termistores::Termistores() {}

/**
 * @brief Inicializa pines MUX y el ADS1115
 * @param sda_pin Pin SDA para I2C (default 21)
 * @param scl_pin Pin SCL para I2C (default 22)
 */
void Termistores::begin(uint8_t sda_pin, uint8_t scl_pin) {
    // Pines MUX como OUTPUT
    pinMode(MUX_S0, OUTPUT);
    pinMode(MUX_S1, OUTPUT);
    pinMode(MUX_S2, OUTPUT);
    pinMode(MUX_S3, OUTPUT);

    // Inicializar I2C para ESP32 (especificando pines)
    Wire.begin(sda_pin, scl_pin);

    if (!ads.begin()) {
        errorFlag = true; 
        // No retorna, permite que se sigan llamando update() pero las lecturas serán NAN.
    }
    ads.setGain(GAIN_ONE); // ±4.096 V
    ads.setDataRate(RATE_ADS1115_860SPS); // Velocidad alta para lecturas rápidas

    // Inicializa el primer canal MUX para que esté listo para la primera lectura
    seleccionarCanal(termistores[0].canalMux);
    previousMuxSwitchTime = millis();
}

/**
 * @brief Función no bloqueante para obtener el error de inicialización
 */
bool Termistores::initError() const {
    return errorFlag;
}

/**
 * @brief Controla los pines del MUX para seleccionar un canal
 */
void Termistores::seleccionarCanal(int canal) {
    // Usamos bitwise operations para mayor eficiencia
    digitalWrite(MUX_S0, canal & 0x01);
    digitalWrite(MUX_S1, (canal >> 1) & 0x01);
    digitalWrite(MUX_S2, (canal >> 2) & 0x01);
    digitalWrite(MUX_S3, (canal >> 3) & 0x01);
}

/**
 * @brief Implementación de Steinhart-Hart y corrección lineal
 * @param rawADC Valor de ADC leído
 * @param t Configuración del termistor
 * @return Temperatura en °C o NAN si hay error
 */
float Termistores::calculateTempC(int16_t rawADC, const Termistor &t) {
    // 1. Convertir ADC a Voltios
    float volts = rawADC * MV_PER_BIT / 1000.0; // mV → V

    // 2. Manejo de errores de lectura/cortocircuito
    if (volts < 0.01) return NAN;

    // 3. Calcular resistencia equivalente
    float Rp = R1 * (VCC / volts - 1.0); // Resistencia del paralelo
    
    // Evitar división por cero o resistencia no válida (ej. circuito abierto)
    if (abs(R2 - Rp) < 0.1) return NAN; 
    
    float Rt = (Rp * R2) / (R2 - Rp);    // Resistencia del termistor

    // 4. Steinhart-Hart
    float logR = log(Rt);
    float tempK = 1.0 / (t.A + t.B * logR + t.C * std::pow(logR, 3.0));
    float tempC = tempK - 273.15; // K → °C

    // 5. Corrección lineal: T_corr = a*T + b
    return t.linA * tempC + t.linB;
}

/**
 * @brief Actualiza la temperatura de un solo termistor de forma no bloqueante (máquina de estados)
 * * Se llama constantemente en loop().
 */
void Termistores::update() {
    // Solo si no hay error de inicialización
    if (errorFlag) return;

    // Revisar si ha pasado el tiempo mínimo de estabilización desde el último cambio de MUX
    if (millis() - previousMuxSwitchTime >= MUX_STABILIZATION_TIME_MS) {
        
        // --- 1. LEER EL TERMISTOR ACTUAL ---
        // Leer el ADC del canal previamente seleccionado (currentTermistorIndex)
        int16_t raw = ads.readADC_SingleEnded(0); 
        
        // Calcular y almacenar la temperatura
        tempsC[currentTermistorIndex] = calculateTempC(raw, termistores[currentTermistorIndex]);

        // --- 2. PREPARAR PARA EL PRÓXIMO TERMISTOR ---
        
        // Avanzar al siguiente índice
        currentTermistorIndex++;
        if (currentTermistorIndex >= N_TERMISTORES) {
            currentTermistorIndex = 0; // Volver al inicio
        }

        // Seleccionar el canal del MUX para la *próxima* lectura
        seleccionarCanal(termistores[currentTermistorIndex].canalMux);

        // Actualizar el tiempo del último cambio (para el próximo ciclo)
        previousMuxSwitchTime = millis();
    }
    // Si no ha pasado el tiempo, simplemente salimos y el loop() puede hacer otras cosas.
}

/**
 * @brief Devuelve la temperatura medida
 */
float Termistores::getTempC(int index) const {
    if (index < 0 || index >= N_TERMISTORES) return NAN;
    return tempsC[index];
}