#include <Arduino.h>
#include "Celdas.h"

// ---------- Objeto ÚNICO ----------
HX711 scale;

// ---------- Pines de Conexión ----------
#define MUX1_A  19
#define MUX1_B  18
#define MUX1_C  5
#define MUX1_D  17
#define MUX2_A  32
#define MUX2_B  33
#define MUX2_C  26
#define MUX2_D  23
#define COMMON_DOUT_PIN 4
#define COMMON_SCK_PIN  16

// --- Canales ---
#define SENSOR_1_CANAL 15
#define SENSOR_2_CANAL 14
#define SENSOR_3_CANAL 13
#define SENSOR_4_CANAL 12
#define SENSOR_5_CANAL 11
#define SENSOR_6_CANAL 10
#define SENSOR_7_CANAL 9
#define SENSOR_8_CANAL 8

const int canales[NUM_CELDAS] = {15,14,13,12,11,10,9,8};

// ---------- Calibración ----------
const float CAL[NUM_CELDAS] = {
  44.647663, 45.054718, 44.557652, 44.690796,
  43.551414, 43.750851, 43.353813, 44.294262
};

long tare_offset[NUM_CELDAS] = {0};

float smoothed[NUM_CELDAS] = {0};
const float ALPHA = 0.08;

const int mux1_pins[] = {MUX1_A, MUX1_B, MUX1_C, MUX1_D};
const int mux2_pins[] = {MUX2_A, MUX2_B, MUX2_C, MUX2_D};

// ---------- Selección de canal ----------
void selectSensor(int channel) {

  for (int i = 0; i < 4; i++) {
    digitalWrite(mux1_pins[i], (channel >> i) & 0x01);
    digitalWrite(mux2_pins[i], (channel >> i) & 0x01);
  }

  delay(3);  // tiempo de asentamiento del mux
}


// void doTare() {
//     Serial.println("🔄 Re-tareando...");

//     const int NUM_SAMPLES = 200;      // tu valor original
//     const int SETTLE_MS = 8;          // tu valor original
//     const unsigned long TIMEOUT_MS = 300;  // tiempo máx para que la celda responda

//     for (int i = 0; i < NUM_CELDAS; i++) {

//         selectSensor(canales[i]);
//         delay(SETTLE_MS);

//         // Antes de leer, asegurarnos de que el HX711 responde
//         if (!scale.wait_ready_timeout(TIMEOUT_MS)) {
//             Serial.printf("⚠️ Celda %d NO responde, se mantiene el offset previo (%ld)\n",
//                           i, tare_offset[i]);
//             continue;  // NO intentamos leer samples → seguimos a la siguiente
//         }

//         long sum = 0;
//         bool lectura_valida = true;

//         for (int k = 0; k < NUM_SAMPLES; k++) {

//             // Si en medio del muestreo se cae la celda → abortamos
//             if (!scale.wait_ready_timeout(TIMEOUT_MS)) {
//                 lectura_valida = false;
//                 break;
//             }

//             long raw = scale.read();

//             // chequeo rápido de lecturas rotas (overflow típico del HX711)
//             if (raw == 0 || raw == 0x7FFFFF || raw == 0x800000) {
//                 lectura_valida = false;
//                 break;
//             }

//             sum += raw;
//             delayMicroseconds(500);
//         }

//         if (!lectura_valida) {
//             Serial.printf("⚠️ Lectura inválida en celda %d, NO se actualiza el tare.\n", i);
//             continue;
//         }

//         // Si todo OK, actualizar offset
//         tare_offset[i] = sum / NUM_SAMPLES;
//         smoothed[i] = 0;

//         Serial.printf("✅ Celda %d tareada correctamente. Offset = %ld\n", i, tare_offset[i]);
//     }

//     Serial.println("✔️ Tare completo (con manejo de fallas).");
// }

void doTare() {
    Serial.println("🔄 Re-tareando (una por una)...");

    const int NUM_AVG = 50;   // igual que tu ejemplo

    for (int i = 0; i < NUM_CELDAS; i++) {

        // Selecciono el canal correcto del multiplexor
        selectSensor(canales[i]);

        // Leo el promedio para tare
        tare_offset[i] = scale.read_average(NUM_AVG);

        // Inicializo el valor suavizado
        // (si usás calibración individual)
        #ifdef CAL
            smoothed[i] = (scale.read() - tare_offset[i]) / CAL[i];
        #else
            smoothed[i] = scale.read() - tare_offset[i];
        #endif

        Serial.printf("  ➤ Celda %d tareada. Offset = %ld\n", i+1, tare_offset[i]);
    }
    }

// ---------- Inicialización ----------
void initCeldas() {
  Serial.println("⚙️ Iniciando celdas + MUX + HX711");

  pinMode(MUX1_A, OUTPUT); pinMode(MUX1_B, OUTPUT);
  pinMode(MUX1_C, OUTPUT); pinMode(MUX1_D, OUTPUT);
  pinMode(MUX2_A, OUTPUT); pinMode(MUX2_B, OUTPUT);
  pinMode(MUX2_C, OUTPUT); pinMode(MUX2_D, OUTPUT);

  scale.begin(COMMON_DOUT_PIN, COMMON_SCK_PIN);
  scale.power_up();

  Serial.println("HX711 listo.");

  doTare();
}


float smoothed2[NUM_CELDAS] = {0};
const float A1 = 0.3;
const float A2 = 0.1;

void getCellValues(float outValues[NUM_CELDAS]) {

    for (int i = 0; i < NUM_CELDAS; i++) {

        selectSensor(canales[i]); 
        delay(5);  // importantísimo

        long raw = scale.read();
        float val = (raw - tare_offset[i]) / CAL[i];

        // Primer filtro
        smoothed[i] = A1 * val + (1 - A1) * smoothed[i];

        // Segundo filtro
        smoothed2[i] = A2 * smoothed[i] + (1 - A2) * smoothed2[i];

        outValues[i] = smoothed2[i];
    }
}
