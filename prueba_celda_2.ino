// /*
//  ESP32 Wroom-32 + HX711
//  Setup: iniciar sin peso en la balanza, luego colocar peso conocido.
//  Ajustar calibration_factor desde el monitor serial con +/- o a/z.
//  Conexiones:
//    ESP32 GPIO18 -> HX711 CLK
//    ESP32 GPIO19 -> HX711 DOUT
//    ESP32 Vin    -> HX711 VCC
//    ESP32 GND    -> HX711 GND
// */

// #include "HX711.h"

// HX711 scale;  // Se declara sin parámetros

// float calibration_factor = 48100;  // Ajustar según tu celda de carga
// float units;
// float ounces;

// void setup() {
//   Serial.begin(115200);  // ESP32 va cómodo a 115200 baudios
//   Serial.println("ESP32 + HX711 Calibration Sketch");

//   // Inicializar HX711 con pines ESP32: DOUT = 19, CLK = 18
//   scale.begin(19, 18);

//   scale.set_scale();   // Escala en bruto al inicio
//   scale.tare();        // Resetear a 0

//   long zero_factor = scale.read_average(); // lectura base (opcional)
//   Serial.print("Zero factor: ");
//   Serial.println(zero_factor);
// }

// void loop() {
//   scale.set_scale(calibration_factor); // Ajustar el factor de calibración

//   // Tomar promedio de 10 lecturas para más estabilidad
//   units = scale.get_units(10);
//   if (units < 0) {
//     units = 0.00;
//   }

//   ounces = units * 0.035274;  // conversión opcional a onzas

//   Serial.print("Peso: ");
//   Serial.print(units);
//   Serial.print(" kg   |   ");
//   Serial.print(ounces);
//   Serial.println(" oz");

//   // Permite ajustar la calibración desde el monitor serie
//   if (Serial.available()) {
//     char temp = Serial.read();
//     if (temp == '+' || temp == 'a')
//       calibration_factor += 1;
//     else if (temp == '-' || temp == 'z')
//       calibration_factor -= 1;

//     Serial.print("Nuevo calibration_factor: ");
//     Serial.println(calibration_factor);
//   }

//   delay(500); // medio segundo entre lecturas
// }

// /*
//  ESP32 Wroom-32 + HX711
//  Funciones:
//    - Lectura de valor bruto (bits)
//    - Lectura de peso calibrado
//    - Tare (poner en cero con 't')
//    - Ajustar calibration_factor con +/- o a/z
//  Conexiones:
//    ESP32 GPIO18 -> HX711 CLK
//    ESP32 GPIO19 -> HX711 DOUT
//    ESP32 Vin    -> HX711 VCC
//    ESP32 GND    -> HX711 GND
// */

// #include "HX711.h"

// HX711 scale;

// float calibration_factor = 48100;  // Ajustar según calibración
// float units;

// void setup() {
//   Serial.begin(115200);
//   Serial.println("ESP32 + HX711 - Lectura de bits y peso");

//   scale.begin(19, 18);   // DOUT = 19, CLK = 18
//   scale.set_scale();
//   scale.tare();          // arranca en cero

//   long zero_factor = scale.read_average(); // valor base en bits
//   Serial.print("Zero factor (bits): ");
//   Serial.println(zero_factor);
// }

// void loop() {
//   scale.set_scale(calibration_factor);

//   // Valor bruto en bits
//   long raw_value = scale.get_value(10);  // promedio 10 lecturas crudas
//   Serial.print("Raw (bits): ");
//   Serial.print(raw_value);

//   // Valor calibrado en "kg"
//   units = scale.get_units(10);  // promedio 10 lecturas calibradas
//   if (units < 0) units = 0.00;
//   Serial.print("   |   Peso: ");
//   Serial.print(units);
//   Serial.println(" kg");

//   // Control por serial
//   if (Serial.available()) {
//     char temp = Serial.read();
//     if (temp == '+' || temp == 'a') {
//       calibration_factor += 1;
//       Serial.print("Nuevo calibration_factor: ");
//       Serial.println(calibration_factor);
//     }
//     else if (temp == '-' || temp == 'z') {
//       calibration_factor -= 1;
//       Serial.print("Nuevo calibration_factor: ");
//       Serial.println(calibration_factor);
//     }
//     else if (temp == 't') {   // Tarar (resetear a cero)
//       scale.tare();
//       Serial.println("Tare realizado: balanza en cero.");
//     }
//   }

//   delay(500);
// }

#include "HX711.h"

HX711 scale;

long raw_value = 0;       // lectura cruda
long zero_offset = 0;     // valor de cero

void setup() {
  Serial.begin(115200);

  // Pines ESP32 (ajustar si usás otro)
  scale.begin(19, 18);
  
  Serial.println("HX711 listo.");
  Serial.println("Presiona 't' para tare (setear cero).");
}

void loop() {
  // Leer valor crudo (en bits)
  raw_value = scale.read();

  // Restarle el offset si ya fue seteado
  long adjusted_value = raw_value - zero_offset;

  // Mostrar en Serial
  Serial.print("Raw: ");
  Serial.print(raw_value);
  Serial.print(" | Ajustado: ");
  Serial.println(adjusted_value);

  delay(200);

  // Escuchar comandos del monitor serie
  if (Serial.available()) {
    char temp = Serial.read();
    if (temp == 't') {  // tare
      zero_offset = raw_value;  // guardo valor actual como cero
      Serial.print("Nuevo cero seteado: ");
      Serial.println(zero_offset);
    }
  }
}

