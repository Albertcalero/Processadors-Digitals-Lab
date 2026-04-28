# Memòria de la Pràctica 3: WIFI I BLUETOOTH

Treball realitzat per: Albert Calero i Alex Navarra Aquesta memoria correspon a la totalitat de la pràctica 3\.

## Objectiu

L’objectiu de la pràctica és comprendre el funcionament de la **\*\*comunicació WiFi\*\*** en l’ESP32 mitjançant:

- La connexió a una xarxa WiFi (mode STA)

- La creació d’un **\*\*servidor web\*\***

- La visualització d’una pàgina web des d’un navegador

---

##  Introducció teòrica

WiFi i Servidor Web

L'ESP32 en mode STA (Station) es connecta a una xarxa existent (com el router de casa).

Un cop connectat, pot actuar com un servidor HTTP (port 80\) que escolta peticions i respon enviant fitxers de text que el navegador interpreta com una pàgina web.

ArduinoOTA (Over-The-Air)

L'OTA és una tecnologia crítica en el món de l'IoT. Permet que el dispositiu quedi instal·lat en un lloc de difícil accés i que el programador pugui enviar noves versions del codi a través de la xarxa WiFi. El codi inclou control d'errors i visualització del progrés de càrrega.

---

##  Pràctica A: Generació d’una pàgina web

###  Descripció

En aquesta pràctica es configura l’ESP32 com a **\*\*client WiFi (mode STA)\*\*** i es crea un **\*\*servidor web\*\*** que respon a peticions HTTP amb una pàgina HTML senzilla.

---

\`\`\`cpp

#include \<Arduino.h\>

#include \<WiFi.h\>

#include \<WebServer.h\>

// SSID & Password

const char\* ssid \= "Nautilus";

const char\* password \= "20000Leguas";

WebServer server(80);

void handle\_root();

void setup()

{

  Serial.begin(115200);

  Serial.println("Intentant connectar a: ");

  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() \!= WL\_CONNECTED)

  {

    delay(1000);

    Serial.print(".");

  }

  Serial.println("\\nWiFi connectat correctament");

  Serial.print("IP assignada: ");

  Serial.println(WiFi.localIP());

  server.on("/", handle\_root);

  server.begin();

  Serial.println("Servidor HTTP iniciat");

}

void loop()

{

  server.handleClient();

}

// Contingut HTML & CSS

String HTML \= R"rawliteral(

  \<\!DOCTYPE html\>

  \<html\>

  \<head\>

  \<meta name="viewport" content="width=device-width, initial-scale=1"\>

  \<style\>

  body {

    font-family: Arial;

    text-align:center;

    color:white;

    background-image:url('https://images.unsplash.com/photo-1506744038136-46273834b3fb');

    background-size:cover;

    background-position:center;

  }

  .box {

    background:rgba(0,0,0,0.6);

    margin:40px auto;

    padding:20px;

    width:80%;

    border-radius:10px;

  }

  img {

    width:200px;

    margin:10px;

    border-radius:10px;

  }

  \</style\>

  \</head\>

  \<body\>

  \<div class="box"\>

    \<h1\>Mi pagina con ESP32\</h1\>

    \<p\>Esta placa con tantos botones y perifericos instalados encima me recuerda a este bosque.\</p\>

    \<h2\>Imagenes\</h2\>

    \<img src="https://upload.wikimedia.org/wikipedia/commons/7/7e/Espressif\_ESP32\_Chip.jpg"\>

    \<img src="https://images.unsplash.com/photo-1518770660439-4636190af475"\>

    \<p\>Juventud divino tesoro...\</p\>

  \</div\>

  \</body\>

  \</html\>

)rawliteral";

void handle\_root()

{

  server.send(200, "text/html", HTML);

}

\`\`\`

---

###  Funcionament del programa

1\. Es defineixen les credencials de la xarxa WiFi (SSID i password).

2\. L’ESP32 intenta connectar-se a la xarxa.

3\. Quan la connexió és correcta:

   - Es mostra la IP assignada pel port sèrie.

4\. Es crea un servidor web al port 80\.

5\. Quan un usuari accedeix a la IP des d’un navegador:

   - El servidor envia una pàgina HTML.

---

###  Pàgina web generada

La pàgina web és senzilla i conté un missatge de prova:

\`\`\`html

\<\!DOCTYPE html\>

\<html\>

\<body\>

\<h1\>My Primera Pagina con ESP32 - Station Mode 😊\</h1\>

\</body\>

\</html\>

\`\`\`

---

## Part B: Implementació de Manteniment OTA

Aquest segon codi permet que el dispositiu estigui "escoltant" noves actualitzacions de firmware. És una eina de diagnòstic i millora que complementa qualsevol projecte WiFi

### Software

\`\`\`cpp

#include \<WiFi.h\>

#include \<ArduinoOTA.h\>

const char\* ssid \= "Nautilus";

const char\* password \= "20000Leguas";

void setup()

{

  Serial.begin(115200);

  WiFi.begin(ssid, password);

  Serial.print("Conectando a WiFi");

  while (WiFi.status() \!= WL\_CONNECTED)

  {

    delay(500);

    Serial.print(".");

  }

  Serial.println("\\nWiFi conectado");

  Serial.print("IP del ESP32: ");

  Serial.println(WiFi.localIP());

  ArduinoOTA.onStart(\[\]()

  {

    Serial.println("Iniciando OTA...");

  });

  ArduinoOTA.onEnd(\[\]()

  {

    Serial.println("\\nOTA completada");

  });

  ArduinoOTA.onProgress(\[\](unsigned int progress, unsigned int total)

  {

    Serial.printf("Progreso: %u%%\\r", (progress / (total / 100)));

  });

  ArduinoOTA.onError(\[\](ota\_error\_t error)

  {

    Serial.printf("Error\[%u\]: ", error);

    if (error \== OTA\_AUTH\_ERROR) Serial.println("Error de autenticación");

    else if (error \== OTA\_BEGIN\_ERROR) Serial.println("Error al iniciar OTA");

    else if (error \== OTA\_CONNECT\_ERROR) Serial.println("Error de conexión");

    else if (error \== OTA\_RECEIVE\_ERROR) Serial.println("Error al recibir datos");

    else if (error \== OTA\_END\_ERROR) Serial.println("Error al finalizar OTA");

  });

  ArduinoOTA.begin();

}

void loop()

{

  ArduinoOTA.handle(); // Necessari per mantenir l'escolta d'actualitzacions

}

\`\`\`

---

## Resultats

- Servidor Web

Un cop connectat a la xarxa "Nautilus", l'ESP32 imprimeix la seva IP (per exemple 192.168.1.45). En accedir des del navegador, es carrega el bloc HTML amb estils CSS.

S'observa l'ús de la sintaxi R"rawliteral(...)rawliteral" per poder escriure codi HTML de múltiples línies de forma neta dins de C++.

Funcionament OTA

El codi configura els "callbacks" o funcions de resposta per a cada estat del procés (inici, progrés, final i error).

A l'IDE d'Arduino o PlatformIO, el dispositiu apareix a la llista de "Ports" com una adreça de xarxa en lloc d'un port COM/USB.

S'ha verificat que es pot carregar codi nou sense connectar el cable físic.

---

## Conclusions

Aquesta pràctica demostra que el WiFi en l'ESP32 no serveix només per enviar dades a un servidor extern, sinó per convertir el propi microcontrolador en una interfície d'usuari interactiva i en un sistema autònom capaç d'actualitzar-se a distància.

La combinació d'un Servidor Web per a la interacció i OTA per al manteniment formen la base de qualsevol dispositiu IoT professional.

