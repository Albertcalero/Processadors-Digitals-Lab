#include <Arduino.h>

#define LED_G 21    //GPIO del led verde
#define LED_R 2     //GPIO del led rojo
#define T_G 1000    //Periodo del led verde
#define T_R 300     //Periodo del led rojo

void Blink (uint8_t GPIO, uint32_t T);

void taskLED_G( void * parameter );
void taskLED_R( void * parameter );

void setup()
{
  Serial.begin(115200);

  //Definimos tareas
  xTaskCreate(
  taskLED_G, /* Task function. */
  "Tarea de parpadeo del led verde", /* name of task. */
  10000, /* Stack size of task */
  NULL, /* parameter of the task */
  1, /* priority of the task */
  NULL); /* Task handle to keep track of created task */

  xTaskCreate(
  taskLED_R, /* Task function. */
  "Tarea de parpadeo del led rojo", /* name of task. */
  10000, /* Stack size of task */
  NULL, /* parameter of the task */
  1, /* priority of the task */
  NULL); /* Task handle to keep track of created task */
}

void loop ()
{

}

void Blink(uint8_t GPIO, uint32_t T)
{
  pinMode(GPIO, OUTPUT);

  while(true)
  {
    digitalWrite(GPIO, HIGH);
    vTaskDelay(T/2);
    digitalWrite(GPIO, LOW);
    vTaskDelay(T/2);
  }
}

void taskLED_G( void * parameter )
{
  Blink(LED_G, T_G);
}

void taskLED_R( void * parameter )
{
  Blink(LED_R, T_R);
}