#include <Arduino.h>

#define LED 2         //GPIO del led
#define T_HIGH 500    //Periodo encendido
#define T_LOW 200     //Periodo apagado

void taskEncender ( void * parameter );
void taskApagar ( void * parameter );

SemaphoreHandle_t semOn;
SemaphoreHandle_t semOff;


void setup()
{
  Serial.begin(115200);

  pinMode(LED, OUTPUT);

  semOn = xSemaphoreCreateBinary();
  semOff = xSemaphoreCreateBinary();
 
  //Definimos tareas
  xTaskCreate(
  taskEncender, /* Task function. */
  "Tarea de encendido del LED", /* name of task. */
  10000, /* Stack size of task */
  NULL, /* parameter of the task */
  1, /* priority of the task */
  NULL); /* Task handle to keep track of created task */

  xTaskCreate(
  taskApagar, /* Task function. */
  "Tarea de apagado del LED", /* name of task. */
  10000, /* Stack size of task */
  NULL, /* parameter of the task */
  1, /* priority of the task */
  NULL); /* Task handle to keep track of created task */

  xSemaphoreGive(semOn);
}

void loop ()
{

}

void taskEncender ( void * parameter )
{
  for(;;)
  {
    if(xSemaphoreTake(semOn, portMAX_DELAY))
    {
      digitalWrite(LED, HIGH);
      Serial.println("LED on.");
      vTaskDelay(T_HIGH / portTICK_PERIOD_MS);

      xSemaphoreGive(semOff); // Da paso a la otra tarea
    }
  }
}

void taskApagar ( void * parameter )
{
  for(;;)
  {
    if(xSemaphoreTake(semOff, portMAX_DELAY))
    {
      digitalWrite(LED, LOW);
      Serial.println("LED off.");
      vTaskDelay(T_LOW / portTICK_PERIOD_MS);

      xSemaphoreGive(semOn); // Da paso a la otra tarea
    }
  }
}