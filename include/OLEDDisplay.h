#ifndef OLEDDISPLAY_H
#define OLEDDISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C

class OledDisplay {
public:
    OledDisplay();
    bool begin(int sda = 21, int scl = 22);
    void showPressure(int index, float value);     // index 0..7
    void showTemperature(int index, float value);  // index 0..9
    void showPaused(const String &ip);
    void showMessage(const String &line1, const String &line2 = "");
private:
    Adafruit_SSD1306 display;
};

#endif
