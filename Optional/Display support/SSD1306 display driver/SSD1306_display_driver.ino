#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

bool blink = true;
int location = 0;

void initializeDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
}

void updateFrame() {
  if (blink) {
    display.drawPixel(location, 10, WHITE);
  } else {
    display.drawPixel(location, 10, BLACK);
  }


  location += 1;
  display.display();

  if (location > 128) {
    location = 0;
    blink = !blink;
  }
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
