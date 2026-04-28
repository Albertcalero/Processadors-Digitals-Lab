#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>

// ================= WIFI =================
const char* ssid = "Nautilus";
const char* password = "20000Leguas";

// ================= LCD =================
#define SDA 8
#define SCL 9
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ================= WEB =================
WebServer server(80);

// ================= TEXTO =================
String textoLCD = "ESP32 + LCD";
String ultimoTexto = "";

// ================= LOGO =================
byte logo[8] = {
  B00100,
  B01110,
  B11111,
  B10101,
  B11111,
  B01110,
  B00100,
  B00000
};

// ======================================================
// HTML
// ======================================================
String paginaHTML()
{
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
  html += "<title>Mi Proyecto</title></head><body>";

  html += "<h1>♥ Mi Proyecto</h1>";
  html += "<form action='/set'>";
  html += "<input type='text' name='msg'>";
  html += "<input type='submit' value='Enviar'>";
  html += "</form>";

  html += "</body></html>";
  return html;
}

// ======================================================
// WEB HANDLERS
// ======================================================
void handleRoot()
{
  server.send(200, "text/html", paginaHTML());
}

void handleSet()
{
  if (server.hasArg("msg"))
  {
    textoLCD = server.arg("msg");
  }

  // 🔁 REDIRECCIÓN AUTOMÁTICA
  server.sendHeader("Location", "/");
  server.send(303);
}

// ======================================================
// SETUP
// ======================================================
void setup()
{
  Serial.begin(115200);

  Wire.begin(SDA, SCL);

  lcd.init();
  lcd.backlight();
  lcd.createChar(0, logo);

  // Pantalla fija (solo una vez)
  lcd.setCursor(0, 0);
  lcd.write(byte(0));
  lcd.print(" Mi Proyecto");

  // ---- WIFI ----
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nIP:");
  Serial.println(WiFi.localIP());

  // ---- WEB ----
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
}

// ======================================================
// LOOP
// ======================================================
void loop()
{
  server.handleClient();

  if (textoLCD != ultimoTexto)
  {
    lcd.clear();

    // ---- Línea 0 (fija) ----
    lcd.setCursor(0, 0);
    lcd.write(byte(0));
    lcd.print(" Mi Proyecto");

    // ---- Línea 1 ----
    lcd.setCursor(0, 1);
    lcd.print(textoLCD.substring(0, 20));

    // ---- Línea 2 ----
    if (textoLCD.length() > 20)
    {
      lcd.setCursor(0, 2);
      lcd.print(textoLCD.substring(20, 40));
    }

    // ---- Línea 3 ----
    if (textoLCD.length() > 40)
    {
      lcd.setCursor(0, 3);
      lcd.print(textoLCD.substring(40, 60));
    }

    ultimoTexto = textoLCD;
  }
}