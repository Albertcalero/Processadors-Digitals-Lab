#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN    48
#define LED_COUNT  1
#define DELAY_MS   500

Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Colores RGBCMYW
uint32_t colores[] = {
  led.Color(255,   0,   0),   // R
  led.Color(0,   255,   0),   // G
  led.Color(0,     0, 255),   // B
  led.Color(0,   255, 255),   // C
  led.Color(255,   0, 255),   // M
  led.Color(255, 255,   0),   // Y
  led.Color(255, 255, 255)    // W
};

void setup() {
  led.begin();
  led.setBrightness(50);   // Ajusta brillo (0–255)
}

void loop() {
  for (int i = 0; i < 7; i++) {
    led.setPixelColor(0, colores[i]);
    led.show();
    delay(DELAY_MS);
  }
}