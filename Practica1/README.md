# Informe de la Pràctica 1: Jugant amb el LED de l'ESP32

Treball realitzat per: Albert Calero i Alex Navarra
Aquesta memoria correspon a la totalitat de la practica 1

## Què volíem aconseguir?
L'objectiu d'aquest primer contacte no era només encendre un llum, sinó entendre com funciona el "cervell" de la nostra placa. Principalment, ens hem centrat en:
* **Entendre el PlatformIO:** Veure que aquí les llibreries no es barregen totes com a l'IDE d'Arduino, sinó que cada projecte té les seves pròpies.
* **Controlar el LED:** Aprendre a manipular el LED RGB que ja ve integrat a la placa[cite: 5, 150].
* **Xafardejar el maquinari:** Saber com llegir les característiques del microcontrolador que tenim connectat al PC.

---

## Amb què hem treballat? (Hardware)
Hem fet servir una placa **ESP32-S3**, que és pràcticament igual al model oficial d'Espressif. 

El millor de la placa és que inclou un **LED RGB tipus NeoPixel**. A diferència dels LEDs normals, aquest es controla amb un sol fil de dades (bus sèrie), cosa que ens facilita molt la vida a l'hora de programar colors.



<img width="1113" height="499" alt="image" src="https://github.com/user-attachments/assets/fe7c48c9-5a9e-456d-9450-56e857995e44" />

<img width="600" height="599" alt="image" src="https://github.com/user-attachments/assets/41af6370-56e2-4244-a6ee-798707934fcc" />

---

## El nostre codi i les proves al laboratori
El programa per encendre el pin no és més que un bucle. Un detall va ser que ha classe vam jugar amb fer proves amb el delay.



```cpp
#include <Arduino.h>

// Pin 2
#define led 17 //  GPIO on es troba el LED.

void setup() 
{                
   pinMode(led, OUTPUT); // Configura el GPIO com a soritda.
  Serial.begin(115200);  
  while(!Serial); // Esperar a que el monitor se abra

  Serial.println("ESP32 iniciado");
}

// Bucle infinit.
void loop() 
{
  digitalWrite(led, HIGH);   // LED on (HIGH es nivell de tensio 5V)
  Serial.println("LED on");  // Escriu per pantalla
  delay(500);                // Espera 0.5s
  digitalWrite(led, LOW);    // LED off (LOW es 0V)
  Serial.println("LED off"); // Escriu per pantalla
  delay(500);                // Espera 0.5s
  //Acesso directo a pin, 4 veces mas rapido
  //GPIO.out_w1ts = (1 << led); // SET pin 
  //GPIO.out_w1tc = (1 << led); // CLEAR pin
}
```

Curiositats: 
Com podem veure als comentaris vam intentar mirar la velocitat màxima de blink d’aquest pin, és a dir quin és el període de canvi d'estat de High a Low.
Primer vam comentar els comandes delay, y vam veure que tenia una velocitat d’1GHz aproximadament.
Llavors ens vam preguntar, i si podem aprofitar millor la velocitat del procesador, i vam trobar aquesta funció GPIO.out_w1tc = (1<<PIN) que canvia d’estat el GPIO de # = PIN

## Pràctica Complementaria: Led RGB
Un cop vam entendre com controlar un LED normal, vam passar a provar el LED RGB integrat a la placa. A diferència d’un LED estàndard, aquest permet controlar diferents colors utilitzant un sol pin de dades.

En aquesta prova vam programar una seqüència de colors (vermell, verd, blau, cian, magenta, groc i blanc) que es mostren un darrere l’altre amb un petit retard.

```cpp
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

```

## Diagrama de flujo del programa

El siguiente diagrama representa el funcionamiento del programa que enciende y apaga el LED de forma repetitiva.

<img width="8192" height="383" alt="image" src="https://github.com/user-attachments/assets/6d5e817e-3b5b-4d03-a8b4-31584d2a9ffc" />
---

## Diagrama de tiempos

El comportamiento temporal del LED es periódico: el LED está encendido durante 500 ms y apagado durante otros 500 ms.

```
Tiempo (ms) →

LED
ON  ──────── 500 ms ────────
      ________
     |        |
_____|        |________________|        |________________

OFF           500 ms
```

Otra forma de representarlo:

| Tiempo (ms) | Estado del LED |
|-------------|---------------|
| 0 – 500     | Encendido     |
| 500 – 1000  | Apagado       |
| 1000 – 1500 | Encendido     |
| 1500 – 2000 | Apagado       |

El periodo total del ciclo es:

```
T = 500 ms + 500 ms = 1000 ms (1 segundo)
```

---

## Tiempo libre del procesador

En el programa utilizado, el microcontrolador pasa la mayor parte del tiempo ejecutando la función `delay()`.

La función `delay(500)` bloquea el programa durante **500 milisegundos**, lo que significa que el procesador no está realizando otras tareas activas del programa durante ese tiempo.

En cada ciclo del `loop()` tenemos:

- `delay(500 ms)` con el LED encendido
- `delay(500 ms)` con el LED apagado

Por lo tanto:

```
Tiempo total del ciclo = 1000 ms
Tiempo en delay = 1000 ms
```

El procesador està **pràcticament lliure el 100% del temps**, ja que la major part del cicle està esperant dins de `delay()`, excepte uns pocs microsegons necessaris per executar instruccions com `digitalWrite()` o `Serial.println()`.

Això significa que el microcontrolador podria dedicar aquest temps a realitzar altres tasques si s’utilitzara una programació **no bloquejant** (per exemple utilitzant `millis()` en lloc de `delay()`).

