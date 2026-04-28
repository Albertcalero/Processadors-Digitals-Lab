#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "Nautilus";
const char* password = "20000Leguas";

void setup() 
{
  Serial.begin(115200);

  // Conectar a WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi conectado");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.onStart([]() 
  {
    Serial.println("Iniciando OTA...");
  });

  ArduinoOTA.onEnd([]() 
  {
    Serial.println("\nOTA completada");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) 
  {
    Serial.printf("Progreso: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) 
  {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Error de autenticación");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Error al iniciar OTA");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Error de conexión");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Error al recibir datos");
    else if (error == OTA_END_ERROR) Serial.println("Error al finalizar OTA");
  });

  ArduinoOTA.begin();
}

void loop() 
{
  ArduinoOTA.handle();
}