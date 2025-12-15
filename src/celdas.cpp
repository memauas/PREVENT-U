#include <Arduino.h>
#include "Celdas.h"

// ---------- Objeto ÚNICO ----------
HX711 scale;

// ---------- Pines de Conexión ----------
#define MUX1_A  19
#define MUX1_B  17
#define MUX1_C  2
#define MUX1_D  15
#define MUX2_A  32
#define MUX2_B  33
#define MUX2_C  26
#define MUX2_D  21
#define COMMON_DOUT_PIN 4
#define COMMON_SCK_PIN  16

// --- Canales ---
#define SENSOR_1_CANAL 0
#define SENSOR_2_CANAL 1
#define SENSOR_3_CANAL 2
#define SENSOR_4_CANAL 3
#define SENSOR_5_CANAL 4
#define SENSOR_6_CANAL 5
#define SENSOR_7_CANAL 6
#define SENSOR_8_CANAL 7
#define SENSOR_9_CANAL 8
#define SENSOR_10_CANAL 9

const int canales[NUM_CELDAS] = {0,1,2,3,4,5,6,7,8,9};

// ---------- Calibración ----------
const float CAL[NUM_CELDAS] = {
  44.647663, 45.054718, 44.557652, 44.690796,
  43.551414, 43.750851, 43.353813, 44.294262, 
  45.548629, 45.519644};



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

  delay(1);  // tiempo de asentamiento del mux
}


void doTare() {
    Serial.println("🔄 Re-tareando (una por una)...");

    const int NUM_AVG = 50;   

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

