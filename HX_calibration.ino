//
//    FILE: HX_calibration.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: HX711 calibration finder for offset and scale
//     URL: https://github.com/RobTillaart/HX711


#include "HX711.h"

HX711 myScale;

// Pines para ESP32 Wroom-32
const int dataPin = 19;   // DT
const int clockPin = 18;  // SCK


void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("HX711_LIB_VERSION: ");
  Serial.println(HX711_LIB_VERSION);
  Serial.println();

  myScale.begin(dataPin, clockPin);
}

void loop()
{
  calibrate();
}


void calibrate()
{
  Serial.println("\n\Calibración\n===========");
  Serial.println("Sacar todo el peso de la celda");
  //  flush Serial input
  while (Serial.available()) Serial.read();

  Serial.println("Presionar enter\n");
  while (Serial.available() == 0);

  Serial.println("Determinar el offset de cero");
  //  Promedio de 50 muestras
  myScale.tare(50);
  int32_t offset = myScale.get_offset();

  Serial.print("OFFSET: ");
  Serial.println(offset);
  Serial.println();

  Serial.println("Poner un peso conocido en la celda");
  //  flush Serial input
  while (Serial.available()) Serial.read();

// Lee el número que pongas en el serial
  Serial.println("Poner el peso en gramos y presionar enter");
  uint32_t weight = 0;
  while (Serial.peek() != '\n')
  {
    if (Serial.available())
    {
      char ch = Serial.read();
      if (isdigit(ch))
      {
        weight *= 10;
        weight = weight + (ch - '0');
      }
    }
  }
  Serial.print("WEIGHT: ");
  Serial.println(weight);
  myScale.calibrate_scale(weight, 50);
  // Lee el valor raw asociado a la deformación
  float scale = myScale.get_scale();

  Serial.print("SCALE:  ");
  Serial.println(scale, 6);

  Serial.print("\nPoner scale.set_offset(");
  Serial.print(offset);
  Serial.print("); and scale.set_scale(");
  Serial.print(scale, 6);
  Serial.print(");\n");
  Serial.println("en el setup de tu proyecto");

  Serial.println("\n\n");
}


//  -- END OF FILE --
