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
const double R_SERIE_CALIBRADA = 10000.0;
const double R_PARALELA = 10000.0;
const double ADC_MAX = 4095.0;

struct SteinhartCoefs { float A, B, C; };
SteinhartCoefs configs[16];


float offsetsTemp[16] = {
 0,0,0,0,0,
    2,            // canal 6
    0.73,            // canal 7
    2,            // canal 8
    1.5,           // canal 9
    -0.15,           // canal 10
    0.7,            // canal 11
    -2,            // canal 12
    1.09,            // canal 13
    1.20,            // canal 14
    1.3             // canal 15
};

void initTermistores() {
    pinMode(S0_T, OUTPUT);
    pinMode(S1_T, OUTPUT);
    pinMode(S2_T, OUTPUT);
    pinMode(S3_T, OUTPUT);
    pinMode(SIG_T, INPUT);
   
    // Tus coeficientes calibrados
    configs[13] = {0.001193349566522334, 0.0002166857413151746, 2.3045800967282677e-7};
    configs[10] = {0.0011343204985665573, 0.00022466165275112574, 1.979051797627113e-7};
    configs[8]  = {0.0012102788297408103, 0.00020975924823460253, 2.8586948453119277e-7};
    configs[11] = {0.0013356592011885891, 0.00018897383921622952, 3.643167584255805e-7};
    configs[15] = {0.0015191263353988007, 0.00015363037614426922, 5.466478097450368e-7};
    configs[9]  = {0.001552217686568973, 0.00015071917696941108, 5.333542706722855e-7};
    configs[14] = {0.0010279078574312555, 0.00024405713599303747, 1.192346379913156e-7};
    configs[12] = {0.0013037425676406368, 0.00019372037307783065, 3.5244096442775185e-7};
    configs[7]  = {0.0012276027733045497, 0.00021069085776341236, 2.5279663334087364e-7};
    configs[6]  = {0.0014856675620528422, 0.00016446981352987453, 4.651435166769681e-7};

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


    temp += offsetsTemp[canal];

    return temp;
}

void leerTodasLasTemperaturas(float outTemps[10]) {
    for(int i = 0; i < 10; i++) {
        int canalReal = canalesTermistores[i];
        outTemps[i] = leerTemperaturaCanal(canalReal);
    }
}
