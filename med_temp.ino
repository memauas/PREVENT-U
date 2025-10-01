// Lectura de termistor NTC 10k con ESP32
// Circuito: divisor resistivo 10k + NTC 10k

const int ntcPin = 36;   // Pin ADC donde conectaste VDIV OUT
const float Vcc = 3.3;   // Alimentación ESP32
const int ADC_MAX = 4095; // Resolución de 12 bits ESP32

// Valores del termistor NTC
const float Rref = 10000.0; // Resistencia fija 10k
const float Beta = 3950.0;  // Constante Beta (ajustar según datasheet)
const float Tref = 298.15;  // 25°C en Kelvin
const float R0 = 10000.0;   // Resistencia a 25°C

void setup() {
  Serial.begin(115200);
}

void loop() {
  int adcValue = analogRead(ntcPin);
  if (adcValue > 0) {
  float Vout = (adcValue * Vcc) / ADC_MAX;
  float Rntc = (Rref * Vout) / (Vcc - Vout);
  float tempK = 1.0 / ( (1.0 / Tref) + (1.0 / Beta) * log(Rntc / R0) );
  float tempC = tempK - 273.15;
  Serial.print("Temp: "); Serial.println(tempC, 2);
} else {
  Serial.println("Sensor desconectado o lectura inválida");
}

  // Calcular voltaje
  float Vout = (adcValue * Vcc) / ADC_MAX;

  // Calcular resistencia del NTC
  float Rntc = (Rref * Vout) / (Vcc - Vout);

  // Calcular temperatura usando ecuación Beta
  float tempK = 1.0 / ( (1.0 / Tref) + (1.0 / Beta) * log(Rntc / R0) );
  float tempC = tempK - 273.15;

  Serial.print("ADC: "); Serial.print(adcValue);
  Serial.print("\tVout: "); Serial.print(Vout, 3);
  Serial.print("\tRntc: "); Serial.print(Rntc, 1);
  Serial.print("\tTemp: "); Serial.print(tempC, 2);
  Serial.println(" °C");

  delay(1000);
}

