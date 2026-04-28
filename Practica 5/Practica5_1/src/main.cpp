#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <RTClib.h>
#include <Adafruit_SSD1306.h>

// ================= I2C =================
#define SDA 8
#define SCL 9

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ================= SENSORES =================
Adafruit_AHTX0 aht;
RTC_DS3231 rtc;

// ================= FLAGS =================
bool ahtOK = false;
bool rtcOK = false;

// ======================================================
// ESCÁNER I2C
// ======================================================
void scanI2C()
{
  byte error, address;
  Serial.println("Scanning I2C...");

  for (address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("Dispositivo en 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  Serial.println("Scan terminado\n");
}

// ======================================================
// SETUP
// ======================================================
void setup()
{
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA, SCL);

  Serial.println("\nSistema completo");

  // ---- ESCANEAR ----
  scanI2C();

  // ---- OLED ----
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("Error OLED");
    while (1);
  }

  // ---- AHT10 ----
  if (aht.begin())
  {
    Serial.println("AHT10 OK");
    ahtOK = true;
  }
  else
  {
    Serial.println("Error AHT10");
  }

  // ---- RTC ----
  if (rtc.begin())
  {
    Serial.println("RTC OK");
    rtcOK = true;

    if (rtc.lostPower())
    {
      Serial.println("RTC sin hora, ajustando...");
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  }
  else
  {
    Serial.println("Error RTC");
  }

  display.clearDisplay();
}

// ======================================================
// LOOP
// ======================================================
void loop()
{
  float temperatura = 0;
  float humedad = 0;

  // ---- LEER AHT10 ----
  if (ahtOK)
  {
    sensors_event_t hum, temp;
    aht.getEvent(&hum, &temp);

    temperatura = temp.temperature;
    humedad = hum.relative_humidity;
  }

  // ---- LEER RTC ----
  int hora = 0, minuto = 0, segundo = 0;

  if (rtcOK)
  {
    DateTime now = rtc.now();
    hora = now.hour();
    minuto = now.minute();
    segundo = now.second();
  }

  // ---- SERIAL ----
  Serial.println("------ DATOS ------");
  Serial.print("Temp: "); Serial.print(temperatura); Serial.println(" C");
  Serial.print("Hum: "); Serial.print(humedad); Serial.println(" %");
  Serial.print("Hora: ");
  Serial.print(hora); Serial.print(":");
  Serial.print(minuto); Serial.print(":");
  Serial.println(segundo);
  Serial.println("-------------------\n");

  // ---- OLED ----
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  // Hora grande
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(hora);
  display.print(":");
  if (minuto < 10) display.print("0");
  display.print(minuto);

  // Temperatura
  display.setTextSize(1);
  display.setCursor(0, 30);
  display.print("Temp: ");
  display.print(temperatura);
  display.print(" C");

  // Humedad
  display.setCursor(0, 45);
  display.print("Hum: ");
  display.print(humedad);
  display.print(" %");

  display.display();

  delay(1000);
}