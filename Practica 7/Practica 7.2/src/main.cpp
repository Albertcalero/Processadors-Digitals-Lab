#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

constexpr i2s_port_t SPEAKER_PORT = I2S_NUM_0;
constexpr i2s_port_t MIC_PORT = I2S_NUM_1;

// MAX98357A: se mantienen los pines del ejercicio anterior.
constexpr int SPEAKER_BCLK = 4;
constexpr int SPEAKER_LRC = 5;
constexpr int SPEAKER_DOUT = 6;

// INMP441: bus I2S de entrada independiente.
constexpr int MIC_BCLK = 7;   // SCK / BCLK
constexpr int MIC_LRC = 15;   // WS / LRCL
constexpr int MIC_DOUT = 16;  // SD / DOUT del microfono

constexpr uint32_t SAMPLE_RATE = 16000;
constexpr uint32_t RECORD_SECONDS = 2;
constexpr size_t SAMPLE_COUNT = SAMPLE_RATE * RECORD_SECONDS;
constexpr size_t CHUNK_SAMPLES = 256;

// El INMP441 entrega muestras de 24 bits en palabras de 32 bits.
// Desplazar 14 bits deja la senal en 16 bits con una ganancia moderada.
constexpr int MIC_TO_SPEAKER_SHIFT = 14;

int16_t recording[SAMPLE_COUNT];
int32_t micBuffer[CHUNK_SAMPLES];
int16_t speakerBuffer[CHUNK_SAMPLES * 2];

int16_t clampToInt16(int32_t value) {
  if (value > INT16_MAX) {
    return INT16_MAX;
  }
  if (value < INT16_MIN) {
    return INT16_MIN;
  }
  return static_cast<int16_t>(value);
}

void installSpeakerI2S() {
  i2s_config_t config;
  memset(&config, 0, sizeof(config));
  config.mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX);
  config.sample_rate = SAMPLE_RATE;
  config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT;
  config.communication_format =
      static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_STAND_I2S);
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
  config.communication_format =
      static_cast<i2s_comm_format_t>(I2S_COMM_FORMAT_STAND_I2S);
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
  Serial.println("  L/R      -> GND para usar canal izquierdo");
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
          sample == INT16_MIN ? INT16_MAX : abs(static_cast<int>(sample));
      peak = max(peak, absSample);
      energy += static_cast<int64_t>(sample) * sample;
      ++reportSamples;
    }

    if (millis() - lastReport >= 250) {
      const float progress = 100.0f * captured / SAMPLE_COUNT;
      const float rms =
          reportSamples == 0 ? 0.0f : sqrt(static_cast<double>(energy) / reportSamples);
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
