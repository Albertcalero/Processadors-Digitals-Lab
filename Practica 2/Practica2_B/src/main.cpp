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