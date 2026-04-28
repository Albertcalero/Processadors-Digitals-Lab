#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// SSID & Password
const char* ssid = "Nautilus"; // Enter your SSID here
const char* password = "20000Leguas"; //Enter your Password here
WebServer server(80); // Object of WebServer(HTTP port, 80 is defult)

void handle_root();

void setup() 
{
  Serial.begin(115200);
  Serial.println("Try Connecting to ");
  Serial.println(ssid);

  // Connect to your wi-fi modem
  WiFi.begin(ssid, password);

  // Check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED)
  { 
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected successfully");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP()); //Show ESP32 IP on serial
  server.on("/", handle_root);
  server.begin();
  Serial.println("HTTP server started");
  delay(100);
}

void loop() 
{
  server.handleClient();
}

// HTML & CSS contents which display on web server
String HTML = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
  body
  {
    font-family: Arial;
    text-align:center;
    color:white;
    background-image:url('https://images.unsplash.com/photo-1506744038136-46273834b3fb');
    background-size:cover;
    background-position:center;
  }

  .box
  {
    background:rgba(0,0,0,0.6);
    margin:40px auto;
    padding:20px;
    width:80%;
    border-radius:10px;
  }

  img
  {
    width:200px;
    margin:10px;
    border-radius:10px;
  }
  </style>
  </head>

  <body>

  <div class="box">
  <h1>Mi pagina con ESP32</h1>
  <p>Esta placa con tantos botones y perifericos instalados encima me recuerda a este bosque.</p>

  <h2>Imagenes</h2>

  <img src="https://upload.wikimedia.org/wikipedia/commons/7/7e/Espressif_ESP32_Chip.jpg">
  <img src="https://images.unsplash.com/photo-1518770660439-4636190af475">

  <p>Juventud divino tesoro...</p>
  </div>

  </body>
  </html>
)rawliteral";

void handle_root() 
{
  server.send(200, "text/html", HTML);
}