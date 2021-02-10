#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

void initializeDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
}

void updateFrame() {
  // I want to get the menu items here, and then render it for this display
  MenuItem * m = getMenu();
  /*
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(getMenu()[0].getDisplayName());
  */
}

void firmwareUpdateStart() {
  display.clearDisplay();

  display.setCursor(0,0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("Updating firmware...");

  display.display();
}

void firmwareUpdateOnProgress(int progress) {
  display.fillRect(12, 30, 102, 10, WHITE);
  display.fillRect(13 + progress, 31, 100 - progress, 8, BLACK);

  display.display();
}
