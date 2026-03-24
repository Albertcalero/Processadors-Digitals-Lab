#include <Arduino.h>

#include <Arduino.h>

// ================= PINES =================
#define LED_PIN     3
#define BTN_UP      5
#define BTN_DOWN    13

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