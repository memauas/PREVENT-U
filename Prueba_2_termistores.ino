// Pines de control del 74HC4067
#define S0 19
#define S1 21
#define S2 22
#define S3 23

// Pin común del multiplexor conectado al ADC
#define Zpin 34   // ADC1_CH6 (puedes elegir otro ADC válido en ESP32)

// Parámetros del divisor resistivo
const float Rref = 10000.0; // Resistencia fija de referencia (10kΩ típica)

// Constantes Steinhart-Hart para el termistor 1 (C0)
const float A1_TH1 = 0.001334040357135646;
const float B1_TH1 = 0.00019932862976098856;
const float C1_TH1= 2.2433104439267812e-7;

// Constantes Steinhart-Hart para el termistor 2 (C1)
const float A2_TH2 = 0.0015459667425181995;
const float B2_TH2 = 0.00015540525913229787;
const float C2_TH2 = 4.7330825626812253e-7;

// -------- Funciones --------

// Seleccionar canal del multiplexor
void selectChannel(int channel) {
  digitalWrite(S0, channel & 0x01);
  digitalWrite(S1, (channel >> 1) & 0x01);
  digitalWrite(S2, (channel >> 2) & 0x01);
  digitalWrite(S3, (channel >> 3) & 0x01);
  delayMicroseconds(10); // pequeño retardo para estabilizar
}

// Medir resistencia del termistor
float readResistance() {
  int adcValue = analogRead(Zpin);
  float Vout = (adcValue / 4095.0) * 3.3; // ESP32 ADC 12 bits, Vref = 3.3V
  float Rth = (Rref * Vout) / (3.3 - Vout);
  return Rth;
}

// Calcular temperatura usando Steinhart-Hart
float steinhart(float R, float A, float B, float C) {
  float logR = log(R);
  float invT = A + B*logR + C*pow(logR, 3);
  float T = (1.0 / invT) - 273.15; // Kelvin → °C
  return T;
}

void setup() {
  Serial.begin(115200);

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);

  pinMode(Zpin, INPUT);
}

void loop() {
  // Leer Termistor 1 en C0
  selectChannel(0);  
  float R1 = readResistance();
  float T1 = steinhart(R1, A1_TH1, B1_TH1, C1_TH1);

  // Leer Termistor 2 en C1
  selectChannel(1);  
  float R2 = readResistance();
  float T2 = steinhart(R2, A2_TH2, B2_TH2, C2_TH2);

  Serial.print("Termistor 1 (C0): ");
  Serial.print(T1);
  Serial.println(" °C");

  Serial.print("Termistor 2 (C1): ");
  Serial.print(T2);
  Serial.println(" °C");

  Serial.println("------------------");

  delay(2000); // espera 2s antes de la próxima lectura
}
