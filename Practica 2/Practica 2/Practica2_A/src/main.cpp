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