# Memòria de la Pràctica 2: Interrupcions


Treball realitzat per: Albert Calero i Alex Navarra Aquesta memoria correspon a la totalitat de la practica 2
## Objectiu


L’objectiu d’aquesta pràctica és comprendre el funcionament de les **interrupcions** en sistemes encastats, concretament en l’**ESP32**, mitjançant:


- El control de LEDs de forma periòdica.
- L’ús d’una entrada (botó) per modificar el comportament.
- La implementació d’interrupcions tant per **GPIO** com per **temporitzador (timer)**.


---


## Introducció teòrica


### 🔹 Què és una interrupció?


Una **interrupció** és un mecanisme que permet al microprocessador **interrompre l’execució normal** per atendre un esdeveniment urgent mitjançant una funció especial anomenada:


- ISR (*Interrupt Service Routine*)


Un cop atesa la interrupció, el programa continua des del punt on s’havia aturat.


---
## Hardware al laboratori

<img width="410" height="544" alt="Sin título" src="https://github.com/user-attachments/assets/9034a490-e019-484b-93df-19ba716fb6b2" />

---
## El nostre codi i les proves al laboratori


Aquesta pràctica consta de Part A, B i complementaria.


##  Pràctica A: Interrupció per GPIO


**Què es demanava:**


- Configurar un **botó com a entrada** en un pin GPIO.
- Associar una **interrupció hardware** al pin.
- Crear una **ISR** que incrementi un comptador quan es prem el botó.
- Mostrar el nombre de pulsacions pel **port sèrie**.
- Desactivar la interrupció després d’un temps (1 minut).


Per fer-ho, hem utilitzat aquest codi:




```cpp
#include <Arduino.h>


struct Button
{
  const uint8_t PIN;                    //GPIO del boton
  volatile uint32_t numberKeyPresses;   //Contador de pulsaciones
  volatile bool pressed;                //Pulsado o no pulsado
  volatile uint32_t lastInterruptTime;  //Tiempo desde la ultima interrupcion
};


Button button1 = {18, 0, false};        //Creacion del boton


//Funcion para pulsar con filtro antirrebote
//Funcion ATTR que va con las interrupciones
void IRAM_ATTR isr()
{
  uint32_t currentTime = millis();                      //Creem una variable per veure el temps que ha pasat


  // Filtro antirrebote de 200 ms                      
  if (currentTime - button1.lastInterruptTime > 200)    //Esperem a que pasin 200ms
  {
    button1.numberKeyPresses++;                         //sumem 1 al comptador
    button1.pressed = true;                             //posem el boto en pulsat
    button1.lastInterruptTime = currentTime;            //Igualem l'ultima interrupcio al temps que ha pasat
  }
}


void setup()
{
  Serial.begin(115200);                         //Monitor serie del ESP32
  pinMode(button1.PIN, INPUT_PULLUP);           //Posem el pin del boto creat en mode imput pullup
  attachInterrupt(button1.PIN, isr, FALLING);   //La GPIO del boto quan s'ativi fara una interrupcio al sistema
}


void loop()
{
  if (button1.pressed)
  {
    noInterrupts();                                         //Desactiva las interrupciones de la ESP32
    uint32_t presses = button1.numberKeyPresses;            //Iguala una variable al contador de pulsaciones
    button1.pressed = false;                                //Pone el boton en no pulsado
    interrupts();                                           //Activa de nuevo las interrupciones


    Serial.printf("Button pressed %u times\n", presses);    //Escribe las veces que se ha pulsado
  }
}




```
---


## Pràctica B: Interrupció per Timer


**Què es demanava:**


- Configurar un **temporitzador intern** de l’ESP32.
- Generar una **interrupció periòdica** (cada segon).
- Crear una **ISR** que incrementi un comptador.
- Utilitzar correctament `volatile` i **seccions crítiques**.
- Mostrar el recompte d’interrupcions pel **port sèrie**.


Per fer-ho, hem utilitzat aquest codi:


```cpp


#include <Arduino.h>


volatile int interruptCounter;                        //contador volatil, check de interrupcion


int totalInterruptCounter;                            //contador de interrupciones total


hw_timer_t * timer = NULL;                            //temporizador


portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED; //inicializacion del temporizador


//Funcion sincrona con las interrupciones y el temporizador
void IRAM_ATTR onTimer()
{
  portENTER_CRITICAL_ISR(&timerMux);              
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}
void setup()
{
  Serial.begin(115200);
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000, true);
  timerAlarmEnable(timer);
}
void loop()
{
  if (interruptCounter > 0)
  {
    portENTER_CRITICAL(&timerMux);
    interruptCounter--;
    portEXIT_CRITICAL(&timerMux);
    totalInterruptCounter++;
    Serial.print("An interrupt as occurred. Total number: ");
    Serial.println(totalInterruptCounter);
  }
}




```
---


##  Diferència clau


- **Pràctica A (GPIO)** → interrupció provocada per l’usuari (botó)  
- **Pràctica B (Timer)** → interrupció automàtica i periòdica  




---


# Pràctica Complementària: Interrupcions i Codi amb IA




## Objectiu


Desenvolupar un programa per a **ESP32 en PlatformIO** que:


- Controli un **LED** que parpelleja.
- Utilitzi **dos pulsadors** per modificar la freqüència:
  - Un pulsador → augmenta la freqüència.
  - L’altre pulsador → disminueix la freqüència.
- Utilitzi una **interrupció per timer**.
- Implementi **filtrat de rebots (debounce)** en els pulsadors.


---


## Hardware al laboratori


<img width="595" height="619" alt="Sin título2" src="https://github.com/user-attachments/assets/51b60e50-b4c5-448c-b027-2262aa62dba1" />


---


## Funcionament del programa


El programa es basa en una **interrupció periòdica generada per un temporitzador**. Aquesta interrupció s’executa cada cert temps i és la responsable de gestionar:


- El parpelleig del LED
- La lectura dels pulsadors
- El control de la freqüència


---


### Interrupció del timer


- El temporitzador genera interrupcions a intervals regulars.
- A cada interrupció:
  - Es compta el temps.
  - Quan s’arriba al valor establert, es canvia l’estat del LED (ON/OFF).


---


###  Lectura dels pulsadors


- Els pulsadors es llegeixen dins del timer.
- Es detecta si estan premuts.


####  Debounce (filtrat de rebots)


- Quan es detecta una pulsació:
  - Es comprova que hagi passat un temps mínim des de l’última pulsació.
  - Això evita múltiples lectures incorrectes (rebots mecànics).


---


###  Control de la freqüència


- Si es prem el **pulsador d’augment**:
  - Es redueix el temps entre canvis → el LED parpelleja més ràpid.


- Si es prem el **pulsador de disminució**:
  - S’incrementa el temps → el LED parpelleja més lent.


---


###  Control del LED


- El LED canvia d’estat (ON/OFF) dins del timer.
- La velocitat de canvi depèn de la freqüència actual.


---




##  Conclusions


- Les interrupcions permeten un control **precís i eficient**.
- El timer evita l’ús de `delay()`.
- El debounce és essencial per lectures correctes.
- El sistema és escalable per a més funcionalitats.


---


##  Codi necessari


```cpp
#include <Arduino.h>


#include <Arduino.h>


// ================= PINES =================
#define LED_PIN     2
#define BTN_UP      18
#define BTN_DOWN    19


// ================= TIMER =================
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


// ================= VARIABLES =================
volatile uint32_t blinkPeriod = 500;      // periodo en ms
volatile uint32_t counterMs = 0;
volatile bool ledState = false;


// Antirrebote
volatile uint8_t debounceUp = 0;
volatile uint8_t debounceDown = 0;
const uint8_t debounceThreshold = 5;  // 5 ms estables


// ================= ISR =================
void IRAM_ATTR onTimer()
{
    portENTER_CRITICAL_ISR(&timerMux);


    // ---- Control de parpadeo ----
    counterMs++;
    if (counterMs >= blinkPeriod)
    {
        counterMs = 0;
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
    }


    // ---- Lectura pulsador SUBIR ----
    if (digitalRead(BTN_UP) == LOW)
    {
        if (debounceUp < debounceThreshold)
            debounceUp++;
        else
        {
            if (blinkPeriod > 50)
                blinkPeriod -= 50;   // aumenta frecuencia
            debounceUp = 0;
        }
    }
    else
    {
        debounceUp = 0;
    }


    // ---- Lectura pulsador BAJAR ----
    if (digitalRead(BTN_DOWN) == LOW)
    {
        if (debounceDown < debounceThreshold)
            debounceDown++;
        else
        {
            if (blinkPeriod < 2000)
                blinkPeriod += 50;   // baja frecuencia
            debounceDown = 0;
        }
    }
    else
    {
        debounceDown = 0;
    }


    portEXIT_CRITICAL_ISR(&timerMux);
}


// ================= SETUP =================
void setup()
{
    pinMode(LED_PIN, OUTPUT);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);


    // Timer 0, prescaler 80 → 1 tick = 1 µs
    timer = timerBegin(0, 80, true);


    // Interrupción cada 1000 µs (1 ms)
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000, true);
    timerAlarmEnable(timer);
}


// ================= LOOP =================
void loop()
{
    // Vacío: todo funciona por interrupción
}






```




