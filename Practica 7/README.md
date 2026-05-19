# Memòria de la Pràctica 7: Àudio amb I2S

Treball realitzat per: Albert Calero i Alex Navarra


 ## Objectiu
L’objectiu d’aquesta pràctica és comprendre i aplicar el protocol I2S (Inter-IC Sound) per transmetre i capturar dades d’àudio digital amb l’ESP32. Concretament, es busca:

Familiaritzar-se amb la comunicació I2S en sistemes embeguts (mestre–esclau, línies BCLK, WS, DIN/DOUT).
Reproduir un fitxer d’àudio (codificat en AAC) emmagatzemat en memòria flash mitjançant les llibreries AudioGeneratorAAC i AudioOutputI2S.

Connectar i configurar un convertidor I2S-a-analògic (per exemple, el mòdul MAX98357A) per a la sortida de so, i entendre com converteix les mostres digitals en sortida analògica.
Connectar i configurar un micròfon digital I2S per captar àudio i enregistrar-lo en memòria.

Crear un codi que registri 2 segons de so des del micròfon I2S i el reprodueixi després per l’altaveu.


## Introducció teòrica
L’I2S (Inter-IC Sound) és un protocol de comunicació síncron en sèrie dissenyat per transmetre dades d’àudio digital entre dispositius. En un bus I2S típic, un dispositiu mestre genera un rellotge de bits (BCLK) i un rellotge de paraula (WS o LRCLK), mentre que les dades d’àudio es transmeten en sèrie per la línia DIN/DOUT. 

L’ESP32 disposa de dos perifèrics I2S independents, que es poden configurar com a transmissors (TX) o receptors (RX). Cada perifèric I2S pot operar amb DMA dedicat, transmetent o rebent fluxos de dades sense carregar excessivament la CPU.

En un bus I2S estàndard destaquen les línies: BCLK (bit clock), WS (Word Select, indica canal esquerre/dret) i DIN/DOUT (dades en sèrie). Opcionalment, alguns dispositius utilitzen MCLK (rellotge mestre) per referència. L’amplificador MAX98357A és un dispositiu I2S amb DAC i amplificador integrats, de manera que rep el flux digital I2S (BCLK, LRC, DIN) i l’hi converteix a sortida analògica per l’altaveu. 

El micròfon INMP441 és un dispositiu MEMS digital I2S que proporciona mostres PCM pel canal esquerre o dret (segons el pin L/R).

Les llibreries d’àudio disponibles a Arduino (ESP8266Audio de Earle Philhower) faciliten molt aquest procés. Per exemple, AudioGeneratorAAC pot reproduir fitxers AAC mitjançant un decodificador Helix, i AudioOutputI2S envia les mostres resultants a un DAC extern via l’I2S. En essència, es llegeix el fitxer codificat, es decodifica i es transmet a l’I2S sense programar manualment el rellotge.

Aquesta pràctica s’estructura en dues parts: primer reproduirem un fitxer d’àudio AAC guardat en memòria (per exemple el “Nokia Tune”) amb un DAC I2S, i després farem una gravació des del micròfon INMP441 i la reproduirem per l’altaveu. 


## Hardware
Connexions I2S a l’ESP32


![](foto1.jpg)

![](foto2.jpg)



## Pràctica A: Generador de So AAC
En el primer exercici reproduirem un fragment AAC guardat en memòria flash utilitzant les llibreries d’àudio. El codi és el següent:

```cpp
#include "AudioGeneratorAAC.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourcePROGMEM.h"
#include "sampleaac.h"

AudioFileSourcePROGMEM *in;
AudioGeneratorAAC *aac;
AudioOutputI2S *out;

void setup() {
  Serial.begin(115200);
  in = new AudioFileSourcePROGMEM(sampleaac, sizeof(sampleaac));
  aac = new AudioGeneratorAAC();
  out = new AudioOutputI2S();
  out->SetGain(0.125);
  out->SetPinout(36, 35, 12);
  aac->begin(in, out);
}

void loop() {
  if (aac->isRunning()) {
    aac->loop();
  } else {
    aac->stop();
    Serial.printf("Sound Generator\n");
    delay(1000);
  }
}

```
### Funcionament:
 Aquest codi utilitza AudioFileSourcePROGMEM per llegir l’arxiu AAC (sampleaac) emmagatzemat en Flash. L’AudioGeneratorAAC decodifica el fitxer en temps real amb el seu algorisme Helix, i l’AudioOutputI2S transmet les mostres PCM pel bus I2S als pins especificats. Quan acaba la reproducció (s’han enviat totes les mostres), el programa es para i imprimeix "Sound Generator" per sèrie, indicant que el fitxer ha finalitzat.


## Pràctica B: Gravació I2S (INMP441) i Reproducció (MAX98357A)
En aquest exercici configurem dos canals I2S: I2S_NUM_0 per transmissió (TX) i I2S_NUM_1 per recepció (RX). El codi complet és el següent:
```cpp
#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

constexpr i2s_port_t SPEAKER_PORT = I2S_NUM_0;
constexpr i2s_port_t MIC_PORT = I2S_NUM_1;

// MAX98357A
constexpr int SPEAKER_BCLK = 4;
constexpr int SPEAKER_LRC = 5;
constexpr int SPEAKER_DOUT = 6;

// INMP441
constexpr int MIC_BCLK = 7;    // SCK / BCLK
constexpr int MIC_LRC = 15;    // WS / LRCL
constexpr int MIC_DOUT = 16;   // SD / DOUT

constexpr uint32_t SAMPLE_RATE = 16000;
constexpr uint32_t RECORD_SECONDS = 2;
constexpr size_t SAMPLE_COUNT = SAMPLE_RATE * RECORD_SECONDS;
constexpr size_t CHUNK_SAMPLES = 256;

// L'INMP441 entrega mostres de 24 bits en paraules de 32 bits.
// Desplaçar 14 bits deixa la senyal en 16 bits amb una guany moderat.
constexpr int MIC_TO_SPEAKER_SHIFT = 14;

int16_t recording[SAMPLE_COUNT];
int32_t micBuffer[CHUNK_SAMPLES];
int16_t speakerBuffer[CHUNK_SAMPLES * 2];

int16_t clampToInt16(int32_t value) {
  if (value > INT16_MAX) return INT16_MAX;
  if (value < INT16_MIN) return INT16_MIN;
  return static_cast<int16_t>(value);
}

void installSpeakerI2S() {
  i2s_config_t config;
  memset(&config, 0, sizeof(config));
  config.mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX);
  config.sample_rate = SAMPLE_RATE;
  config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  config.communication_format = static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_STAND_I2S);
  config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  config.dma_buf_count = 8;
  config.dma_buf_len = CHUNK_SAMPLES;
  config.use_apll = false;
  config.tx_desc_auto_clear = true;
  config.fixed_mclk = 0;

  i2s_pin_config_t pins;
  memset(&pins, 0, sizeof(pins));
  pins.bck_io_num = SPEAKER_BCLK;
  pins.ws_io_num = SPEAKER_LRC;
  pins.data_out_num = SPEAKER_DOUT;
  pins.data_in_num = I2S_PIN_NO_CHANGE;

  ESP_ERROR_CHECK(i2s_driver_install(SPEAKER_PORT, &config, 0, nullptr));
  ESP_ERROR_CHECK(i2s_set_pin(SPEAKER_PORT, &pins));
  ESP_ERROR_CHECK(i2s_zero_dma_buffer(SPEAKER_PORT));
}

void installMicI2S() {
  i2s_config_t config;
  memset(&config, 0, sizeof(config));
  config.mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX);
  config.sample_rate = SAMPLE_RATE;
  config.bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT;
  config.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
  config.communication_format = static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_STAND_I2S);
  config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  config.dma_buf_count = 8;
  config.dma_buf_len = CHUNK_SAMPLES;
  config.use_apll = false;
  config.tx_desc_auto_clear = false;
  config.fixed_mclk = 0;

  i2s_pin_config_t pins;
  memset(&pins, 0, sizeof(pins));
  pins.bck_io_num = MIC_BCLK;
  pins.ws_io_num = MIC_LRC;
  pins.data_out_num = I2S_PIN_NO_CHANGE;
  pins.data_in_num = MIC_DOUT;

  ESP_ERROR_CHECK(i2s_driver_install(MIC_PORT, &config, 0, nullptr));
  ESP_ERROR_CHECK(i2s_set_pin(MIC_PORT, &pins));
  ESP_ERROR_CHECK(i2s_zero_dma_buffer(MIC_PORT));
}

void printPinout() {
  Serial.println();
  Serial.println("P7 ejercicio 2 - grabar INMP441 y reproducir por MAX98357A");
  Serial.println("Altavoz MAX98357A:");
  Serial.printf("  BCLK -> GPIO%d\n", SPEAKER_BCLK);
  Serial.printf("  LRC  -> GPIO%d\n", SPEAKER_LRC);
  Serial.printf("  DIN  -> GPIO%d\n", SPEAKER_DOUT);
  Serial.println("Microfono INMP441:");
  Serial.printf("  SCK/BCLK -> GPIO%d\n", MIC_BCLK);
  Serial.printf("  WS/LRCL  -> GPIO%d\n", MIC_LRC);
  Serial.printf("  SD/DOUT  -> GPIO%d\n", MIC_DOUT);
  Serial.println("  L/R      -> GND (canal esquerre)");
  Serial.println();
}

void countdownToRecord() {
  Serial.println("Preparado. La grabacion empezara en:");
  for (int i = 3; i > 0; --i) {
    Serial.printf("%d...\n", i);
    delay(1000);
  }
  Serial.println("HABLA AHORA: grabando 2 segundos.");
}

void recordFromMic() {
  ESP_ERROR_CHECK(i2s_zero_dma_buffer(MIC_PORT));

  size_t captured = 0;
  uint32_t lastReport = millis();
  int32_t peak = 0;
  uint64_t energy = 0;
  size_t reportSamples = 0;

  while (captured < SAMPLE_COUNT) {
    const size_t remaining = SAMPLE_COUNT - captured;
    const size_t toRead = min(remaining, CHUNK_SAMPLES);
    size_t bytesRead = 0;

    esp_err_t result = i2s_read(MIC_PORT, micBuffer, toRead * sizeof(int32_t),
                                &bytesRead, portMAX_DELAY);
    if (result != ESP_OK || bytesRead == 0) {
      Serial.printf("Error leyendo microfono: %d\n", result);
      continue;
    }

    const size_t samplesRead = bytesRead / sizeof(int32_t);
    for (size_t i = 0; i < samplesRead && captured < SAMPLE_COUNT; ++i) {
      int16_t sample = clampToInt16(micBuffer[i] >> MIC_TO_SPEAKER_SHIFT);
      recording[captured++] = sample;

      const int32_t absSample =
          (sample == INT16_MIN) ? INT16_MAX : abs(static_cast<int>(sample));
      peak = max(peak, absSample);
      energy += static_cast<int64_t>(sample) * sample;
      ++reportSamples;
    }

    if (millis() - lastReport >= 250) {
      const float progress = 100.0f * captured / SAMPLE_COUNT;
      const float rms = (reportSamples == 0) ? 0.0f :
                        sqrt(static_cast<double>(energy) / reportSamples);
      Serial.printf("Grabando... %5.1f%% | pico: %5ld | rms: %5.0f\n",
                    progress, static_cast<long>(peak), rms);
      lastReport = millis();
      peak = 0;
      energy = 0;
      reportSamples = 0;
    }
  }

  Serial.println("Grabacion terminada.");
}

void waitBeforePlayback() {
  Serial.println("Esperando 2 segundos antes de reproducir.");
  delay(2000);
  Serial.println("Reproduccion en bucle iniciada.");
}

void playRecordingOnce(uint32_t repetition) {
  Serial.printf("Reproduccion #%lu\n", static_cast<unsigned long>(repetition));

  size_t played = 0;
  while (played < SAMPLE_COUNT) {
    const size_t count = min(SAMPLE_COUNT - played, CHUNK_SAMPLES);

    for (size_t i = 0; i < count; ++i) {
      const int16_t sample = recording[played + i];
      speakerBuffer[i * 2] = sample;
      speakerBuffer[i * 2 + 1] = sample;
    }

    size_t bytesWritten = 0;
    ESP_ERROR_CHECK(i2s_write(SPEAKER_PORT, speakerBuffer,
                              count * 2 * sizeof(int16_t), &bytesWritten,
                              portMAX_DELAY));
    played += count;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  printPinout();
  installSpeakerI2S();
  installMicI2S();

  countdownToRecord();
  recordFromMic();
  waitBeforePlayback();
}

void loop() {
  static uint32_t repetition = 1;
  playRecordingOnce(repetition++);
  delay(500);
}

```


### Resultats i Observacions
Reproducció AAC: En executar la Pràctica A amb el DAC MAX98357A connectat, s’hauria d’escoltar clarament el fragment d’àudio (“Nokia Tune”) per l’altaveu. El monitor sèrie mostrarà "Sound Generator", confirmant que l’AudioGeneratorAAC va decodificar i transmetre l’àudio correctament.

Gravació I2S: En la Pràctica B, el monitor sèrie mostra la configuració de pins i després missatges de progrés de gravació  amb valors de pic i RMS. Quan acaba, l’àudio capturat (per exemple, la pròpia veu) es reprodueix per l’altaveu, indicant que l’INMP441 ha enviat correctament les mostres a l’ESP32 i que aquestes es poden reproduir.

Sincronització: No cal gestionar manualment el rellotge BCLK; l’ESP32 I2S s’encarrega de la sincronització de dades. Cal assegurar només les connexions correctes de pins i la configuració del driver, com indiquen les llibreries.

Qualitat d’àudio: A 16 kHz i 16 bits, la qualitat és adequada per veu. L’amplificador MAX98357A funciona amb rangs de voltatge entre 2.7–5.5V, de manera que amb 3.3V alimenta bé un altaveu petit. La configuració mono (L/R units) és la predeterminada i l’ampificador combina els canals automàticament.


## Conclusions
La Pràctica 7 demostra que l’ESP32 pot gestionar fàcilment fluxos d’àudio digitals mitjançant el protocol I2S. Hem comprovat que:

L’I2S transmet dades d’àudio en sèrie amb un rellotge comú (BCLK) i una senyal WS que indica canal, proporcionant un bus eficient per àudio digital. A diferència dels busos paral·lels, l’I2S necessita només 3 línies principals per a àudio bidireccional mestre–esclau.
Les llibreries d’àudio simplifiquen la reproducció: amb unes quantes línies de codi s’ha reproduït un fitxer AAC, ocultant la complexitat del protocol.

La gravació amb un micròfon I2S i la reproducció posterior demostren que l’ESP32 pot actuar simultàniament com a receptor i transmissor I2S, utilitzant DMA dedicat. Això permet escalar a múltiples canals o dispositius en futur.

Limitacions: L’I2S és semiduplex per perifèric (no transmetre i rebre alhora pel mateix canal) i la latència depèn dels búfers DMA. Malgrat això, per aplicacions d’àudio normals (voz, música de baixa banda) ofereix bones prestacions sense haver de cablejar paral·lelament.

En resum, hem comprovat que l’ESP32 pot reproduir i enregistrar àudio digital amb només tres línies de senyal, utilitzant un mòdul MAX98357A per a sortida i un micròfon I2S per a entrada. Aquest coneixement és fonamental per a projectes futurs amb múltiples sensors i sortides d’àudio intel·ligents.


