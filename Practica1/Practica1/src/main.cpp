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