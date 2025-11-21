#include "OledDisplay.h"

static const char* PRESSURE_NAMES[8] = {
    "Met5 Izq", "Met1 Izq", "Met1 Der", "Met5 Der",
    "Side Izq", "Side Der", "Heel Izq", "Heel Der"
};

static const char* TEMP_NAMES[10] = {
    "TalL","LatL","HalluxL","M1L","M5L",
    "TalR","LatR","HallR","M1R","M5R"
};

OledDisplay::OledDisplay()
: display(OLED_WIDTH, OLED_HEIGHT, &Wire) {}

bool OledDisplay::begin(int sda, int scl) {
    Wire.begin(sda, scl);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        return false;
    }
    display.clearDisplay();
    display.setTextSize(1);  // Texto más grande
    display.setTextColor(SSD1306_WHITE);

    // Marca Prevent‑U arriba a la derecha
    display.setCursor(70, 0);
display.setTextSize(1);
    display.println("Prevent-U");

    display.setCursor(0, 20);
    display.println("OLED OK");
    display.display();
    delay(400);
    return true;
}

void OledDisplay::showPressure(int index, float value) {
    display.clearDisplay();
    display.setTextSize(2);

    display.setCursor(70, 0);
display.setTextSize(1);
    display.println("Prevent-U");

    display.setCursor(0, 20);
    display.println("PRESION (kPa)");
    display.setTextSize(1);
    display.println("-------");

    if (index < 0 || index >= 8) {
        display.println("Index fuera de rango");
    } else {
        display.print(PRESSURE_NAMES[index]);
        display.print(": ");
        display.println(value, 2);
    }
    display.display();
}

void OledDisplay::showTemperature(int index, float value) {
    display.clearDisplay();
    display.setTextSize(1);

    display.setCursor(70, 0);
display.setTextSize(1);
    display.println("Prevent-U");

    display.setCursor(0, 20);
    display.println("TEMP (C)");
    display.setTextSize(1);
    display.println("--------");

    if (index < 0 || index >= 10) {
        display.println("Index fuera de rango");
    } else {
        display.print(TEMP_NAMES[index]);
        display.print(": ");
        if (value < -200) {
            display.println("ERROR");
        } else {
            display.println(value, 2);
        }
    }
    display.display();
}

void OledDisplay::showPaused(const String &ip) {
    display.clearDisplay();
    display.setTextSize(1);

    display.setCursor(70, 0);
display.setTextSize(1);
    display.println("Prevent-U");

    display.setCursor(0, 20);
    display.setTextSize(1);
    display.println("MEDICION PAUSADA");
    display.println("---------");
    display.print("IP: ");
    display.println(ip);
    display.println("");
    display.println("Pulsa boton para iniciar");
    display.display();
}

void OledDisplay::showMessage(const String &line1, const String &line2) {
    display.clearDisplay();
    display.setTextSize(1);

    display.setCursor(70, 0);
display.setTextSize(1);
    display.println("Prevent-U");

    display.setCursor(0, 20);
    display.setTextSize(1);
    display.println(line1);
    display.println("---------");
    display.println(line2);
    display.display();
}
