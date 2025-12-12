#include "termistores.h"
#include <math.h>

// Pines del MUX (termistores)
#define S0_T 13
#define S1_T 14
#define S2_T 27
#define S3_T 25
#define SIG_T 34

int canalesTermistores[10] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6};
// Parámetros
const double R_SERIE_CALIBRADA = 8150.0;
const double R_PARALELA = 10000.0;
const double ADC_MAX = 4095.0;

struct SteinhartCoefs { float A, B, C; };
SteinhartCoefs configs[16];


void initTermistores() {
    pinMode(S0_T, OUTPUT);
    pinMode(S1_T, OUTPUT);
    pinMode(S2_T, OUTPUT);
    pinMode(S3_T, OUTPUT);
    pinMode(SIG_T, INPUT);
   
    // Tus coeficientes calibrados
    configs[13] = {0.0003445997, 0.0003633891, -4.478e-7}; //T1 3
    configs[10] = {0.0002314524, 0.0003842840, -5.495e-7}; //T2 6
    configs[8] = {0.0005570546, 0.0003313903, -3.426e-7}; //T3 8
    configs[11] = {-0.0000332643, 0.0004245515, -6.796e-7}; //T4 5
    configs[15] = {0.0005184773, 0.0003382271, -3.688e-7}; //T5 1
    configs[9] = {0.0005088505, 0.0003425892, -4.048e-7}; //T6 7
    configs[14] = {0.0006721010, 0.0003083060, -2.146e-7}; //T7 2
    configs[12] = {0.0003609723, 0.0003621767, -4.409e-7}; //T8 4
    configs[7] = {0.0006409455, 0.0003150994, -2.552e-7}; //T9 10
    configs[6] = {0.0006081904, 0.0003184497, -2.436e-7}; //T10 9

    Serial.println("Termistores inicializados.");
}

void seleccionarCanalT(int canal) {
    digitalWrite(S0_T, canal & 1);
    digitalWrite(S1_T, (canal >> 1) & 1);
    digitalWrite(S2_T, (canal >> 2) & 1);
    digitalWrite(S3_T, (canal >> 3) & 1);
    delayMicroseconds(50);
}

float leerTemperaturaCanal(int canal) {
    seleccionarCanalT(canal);

    long suma = 0;
    for(int i = 0; i < 50; i++) {
        suma += analogRead(SIG_T);
        delay(1);
    }

    double adc = suma / 50.0;
    if (adc < 100 || adc > 4000) return -999.0;

    double R_combo = R_SERIE_CALIBRADA * (ADC_MAX - adc) / adc;
    if (R_combo >= R_PARALELA - 10) return -888.0;

    double R_term = (R_combo * R_PARALELA) / (R_PARALELA - R_combo);

    float logR = log(R_term);
    float temp = (1.0 / (configs[canal].A +
                         configs[canal].B * logR +
                         configs[canal].C * pow(logR, 3))) - 273.15;

    // if (canal == 8) temp += 1;   // T10
    // // if (canal == 0) temp += 0.7;   // T5
    // // if (canal == 1) temp += 0.4;   // T7
    // if (canal == 2) temp -= 0.7;   // T1
    // if (canal == 3) temp += 1.0;   // T8
    // if (canal == 4) temp += 1.2;   // T4
    // // if (canal == 5) temp += 1.3;   // T2
    // // if (canal == 6) temp += 0.7;   // T6
    // if (canal == 7) temp += 0.7;   // T3
    // // if (canal == 9) temp += 1.0;   // T9
    
    return temp;
}

void leerTodasLasTemperaturas(float outTemps[10]) {
    for(int i = 0; i < 10; i++) {
        int canalReal = canalesTermistores[i];
        outTemps[i] = leerTemperaturaCanal(canalReal);
    }
}
