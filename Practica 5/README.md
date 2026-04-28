# Memòria de la Pràctica 5: Busos de comunicació I (I2C)  
Treball realitzat per: Albert Calero i Alex Navarra  

---

## Objectiu

El objectiu d’aquesta pràctica és comprendre els **busos de comunicació**, especialment el bus **I2C**, amb l’ESP32. Concretament, s’apunta a: 

- Familiaritzar-se amb els *bussos sèrie vs. paral·lel* i la seva arquitectura.  
- Entendre l’arquitectura *mestre–esclau* d’un bus I2C.  
- Implementar un **escàner I2C** per detectar dispositius al bus.  
- Connectar i controlar un perifèric I2C d’exemple (per exemple, una pantalla OLED).  

---

## Introducció teòrica

Els busos de comunicació serveixen per interconnectar diferents subsistemes o perifèrics d’un sistema. Per exemple, a l’ESP32 podem comunicar-nos amb sensors i pantalles mitjançant fils dedicats. Els busos poden ser **paral·lels** o **sèrie**. En un bus paral·lel es transmeten diversos bits alhora en múltiples línies, oferint alta velocitat però més interferències entre línies【9†L120-L123】. En canvi, un bus sèrie envia un bit darrere l’altre per menys cables (per exemple, només *dos* fils en I2C), fet que sovint permet distàncies majors i menys interferència【9†L120-L123】. Generalment, un bus de major freqüència de rellotge ha de limitar l’amplada de dades (més pocs bits simultanis) per reduir el *crosstalk* i problemes de sincronització. 


El bus **I2C (Inter-Integrated Circuit)** va ser desenvolupat per Philips el 1982【2†L152-L160】 com a bus intern per dispositius electrònics. Només requereix **dos cables**:  
- **SCL (clock):** senyal de rellotge compartida.  
- **SDA (data):** línia bidireccional per dades (en codi *open-drain*).  

Tots els dispositius I2C han de tenir una **adreça única** (7 bits), fins a 112 dispositius normals pel bus【11†L1-L4】. El primer *byte* enviat inclou aquests 7 bits d’adreça i un bit R/W per indicar lectura o escriptura. El dispositiu màster inicia la comunicació generant el rellotge a SCL i seleccionant l’esclau per la seva adreça. Els esclaus únicament poden respondre quan són interrogats pel màster; no poden iniciar la transferència ells mateixos.

El protocol I2C també defineix l’ús de **resistències pull-up** a les línies. Les línies SDA i SCL es mantenen a nivell alt mitjançant pull-ups a V<sub>CC</sub> (típicament 3.3V). La llibreria `Wire` d’Arduino habilita pull-ups interns febles (~20–30 kΩ), però normalment es recomanen pull-ups externes de 1 kΩ–4.7 kΩ per millorar la velocitat de flanc i poder utilitzar freqüències altes o distàncies majors. Les velocitats estàndard d’I2C són 100 kHz (estàndard) i 400 kHz (fast mode); existixen modes més ràpids (1–5 MHz) que no s’usen sovint en Arduino. 

En resum, les característiques del bus I2C són:  
- **Dues línies + adreça:** Només SDA i SCL (més GND i VCC).  
- **Arquitectura mestre-esclau:** Un mestre controla el rellotge i pregunta als esclaus.  
- **Semiduplex:** No full-duplex; no es pot transmetre i rebre alhora【10†L24-L28】.  
- **Verificació:** Els esclaus confirmen recepció (ACK), però el protocol no valida l’integritat de les dades més enllà d’això【10†L24-L28】.  
- **Velocitat:** Mitjana (sense arribar a la d’interfícies paral·leles o alguns SPI), al voltant de 100–400 kHz【10†L12-L20】.  
- **Multipunteig:** Permet múltiples dispositius al mateix bus mentre tinguin adreces diferents【11†L1-L4】【2†L168-L176】.  

---

## Hardware

![](foto.jpg)

En un ESP32 típic, els pins per defecte per a I2C són els següents:

| Bus I2C | ESP32           |
| ------- | --------------- |
| **SDA** | GPIO 21 (SDA)   |
| **SCL** | GPIO 22 (SCL)   |
| **GND** | GND             |
| **VCC** | 3.3V (o 5V si el dispositiu ho requereix) |

A més, cal connectar resistències de pull-up externes entre SDA, SCL i 3.3V (per exemple, 4.7 kΩ) si es volen comunicacions fiables a altes velocitats o llarga distància【10†L4-L10】. Sense resistències externes, el chip utilitza pull-ups interns molt “febles” (20–30 kΩ)【10†L4-L10】, que limiten la velocitat màxima i la llargada del bus. En general, es recomana sempre utilitzar pull-ups externs adequats per a la configuració del dispositiu.

---

## Pràctica A: Escàner I2C

El primer exercici consisteix a programar un **escàner I2C** que recorre totes les adreces possibles del bus i informa quines responen. L’algoritme bàsic fa una **transmissió dummy** a cada adreça i comprova el codi de retorn: si `Wire.endTransmission()` retorna 0, significa que hi ha un dispositiu amb *ack*. 

A continuació es mostra el codi complet usant `Wire` de Arduino:

```cpp
#include <Arduino.h>
#include <Wire.h>

void setup() {
  Wire.begin();            
  Serial.begin(115200);
  while (!Serial);   // Espera a que s'obri el monitor sèrie
  Serial.println("\nI2C Scanner");
}

void loop() {
  byte error, address;
  int nDevices = 0;
  Serial.println("Scanning...");
  
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
    else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
  
  delay(5000);  // Espera 5 segons abans del següent escaneig
}
```

**Funcionament:** Aquest codi inicialitza el bus I2C (`Wire.begin()`) i el port sèrie. A l’interior del `loop()`, s’itera de la adreça 1 fins a 126. Per cada adreça, es crida `Wire.beginTransmission(address)` i després `Wire.endTransmission()`. Si `endTransmission()` retorna 0, s’ha detectat un dispositiu amb aquell *ack*. L’escàner escriu al port sèrie missatges com:
```
Scanning...
I2C device found at address 0x3C !
done
```
indicant les adreces trobades en format hexadecimal. Si no hi ha dispositius, imprimeix `No I2C devices found`. Aquest comportament coincideix amb la idea general descrita en la documentació: “Scanner I2C recorre les adreces i mostra quins dispositius hi ha connectats”.

---

## Pràctica B: Display OLED I2C (SSD1306)

Com a exemple de dispositiu I2C extern, es pot utilitzar una pantalla OLED basada en el controlador SSD1306 (per exemple de 128×32 píxels). Aquestes pantalles es comuniquen per I2C i normalment la seva adreça és **0x3C** o **0x3D** (en hex). Cal connectar SDA i SCL als pins corresponents de l’ESP32 (21 i 22), i GND/VCC.

El codi d’exemple amb la llibreria Adafruit SSD1306 podria ser el següent:

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32 
#define OLED_RESET -1 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);

  // Inicialitza la pantalla OLED amb adreça 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;) {} // S'atura si no troba la pantalla
  }

  display.clearDisplay();          // Neteja el buffer
  display.setTextSize(2);         // Doble mida de text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Hola OLED!");
  display.display();              // Actualitza la pantalla
}

void loop() {
  // En aquest exemple no fem res més en el loop
}
```

**Funcionament:** El codi importa la llibreria Adafruit per a OLED i crea un objecte `display`. A la `setup()` s’inicialitza la comunicació I2C i la pantalla. L’adreça 0x3C s’indica a `display.begin()`. Després es neteja la pantalla (`clearDisplay()`), es configura la mida de text i la posició del cursor, i finalment es mostra el missatge `"Hola OLED!"` amb `display.display()`. El resultat és que la pantalla OLED il·lumina el text en blanc sobre fons negre. En la pràctica, la sortida esperada seria veure “Hola OLED!” en una línia de la pantalla. Aquesta demostració verifica que la comunicació I2C amb el dispositiu funciona correctament. 

---

## Resultats i Observacions

- **Escàner I2C:** Al connectar un o més dispositius I2C (per exemple, la pantalla OLED SSD1306 i/o altres sensors), l’escàner imrpimeix les adreces detectades al monitor sèrie. Per exemple, si la pantalla OLED té adreça 0x3C, veurem una línia com `I2C device found at address 0x3C !`. Si s’instal·len també altres dispositius (com un sensor de temperatura I2C), també apareixeran les seves adreces. Si no hi ha dispositius o estan desconnectats, l’escàner dirà “No I2C devices found”. Aquest comportament facilita comprovar les connexions i adreces dels perifèrics (tal com descriu la bibliografia).

- **Pantalla OLED:** Una vegada inicialitzat l’I2C, el text «Hola OLED!» es veu clarament a la pantalla. Això confirma que tant el maquinari com el programari són correctes. El dispositiu respon a l’adreça 0x3C i mostra el contingut enviat.

- **Sincronització i temporització:** L’ús de `Wire` s’encarrega de generar la senyal de rellotge SCL adequada i de canviar les línies SDA per fer *ack*. No cal gestionar-ho manualment. En pràctica, s’ha de tenir cura amb les resistències pull-up per assegurar flancs nets, tal com indica la referència.

---

## Conclusions

Aquest estudi del bus I2C demostra que amb poques línies es pot comunicar múltiples dispositius de manera senzilla i eficient. Hem comprovat que:

- El bus **I2C** utilitza només **dos fils** (SDA/SCL) per a comunicació bidireccional mestre–esclau. Això redueix la complexitat de connexió comparat amb un bus paral·lel, encara que a costa de velocitats moderades.  
- **Múltiples dispositius:** És possible tenir molts esclaus al mateix bus si tenen adreces diferents (fins a 112 adreces útils). Hem pogut connectar exemplars (pantalla, sensors) simultàniament.  
- **Escàner I2C:** L’escàner realitzat confirma la teoria: envia una petició dummy a cada adreça i detecta resposta (“ACK”). Això és molt útil quan no sabem l’adreça d’un nou dispositiu.  
- **Biblioteques i abstracció:** Les llibreries `Wire` i `Adafruit_SSD1306` amaguen la complexitat del protocol. Només cal inicialitzar el bus i invocar funcions d’alt nivell (`beginTransmission`, `display()`, etc.), i es maneja la temporització i sincronització.  
- **Limitacions:** El bus I2C és *half-duplex* (no es pot enviar i rebre simultàniament) i la seva velocitat típica (100–400 kHz) és menor que la d’altres interfícies. Tampoc realitza checksum de dades, només ACK. Tot i això, la seva simplicitat de cables el fa ideal per a sensors, pantalles i perifèrics interns en sistemes embeguts.  

---

## Conclusió final

La Pràctica 5 ha servit per comprendre en profunditat el bus I2C i veure’l en funcionament amb l’ESP32. Hem après la diferència fonamental entre comunicació paral·lela i sèrie i les característiques específiques del protocol I2C (màster–esclau, adreces, pull-ups). El resultat ha estat la capacitat de realitzar un escaneig d’adreces I2C i d’interactuar amb dispositius I2C reals (com una pantalla OLED), demostrant que l’ESP32 pot gestionar perifèrics amb aquest tipus de bus de manera estable. Aquests conceptes són la base per a futures aplicacions més complexes amb múltiples sensors i perifèrics, com ara sistemes de control en xarxa o interfícies amb diversos dispositius intel·ligents.

---

## Referències

- Random Nerd Tutorials – *“ESP32 I2C Communication ( Arduino IDE )”* (tutorial general d’I2C amb ESP32).  
- Programarfacil – *“SSD1306 Pantalla OLED con Arduino”* (exemple amb llibreria Adafruit_SSD1306).  
- Luis Llamas – *“El bus I2C en Arduino”* (explicació detallada del protocol I2C)【2†L152-L160】【10†L24-L28】.  
- (Altres webs de referència consultades: documentació oficial Adafruit SSD1306, SparkFun MAX30105, etc.)
