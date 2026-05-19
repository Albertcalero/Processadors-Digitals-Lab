# P7 Ejercicio 2

Graba 2 segundos desde un microfono MEMS INMP441 por I2S, espera 2 segundos y
reproduce la grabacion en bucle por el amplificador I2S MAX98357A.

## Conexiones

Altavoz MAX98357A:

```text
ESP32-S3 GPIO4 -> BCLK
ESP32-S3 GPIO5 -> LRC / WS
ESP32-S3 GPIO6 -> DIN / SD
ESP32-S3 3V3   -> VIN / VCC
ESP32-S3 GND   -> GND
Altavoz +      -> salida + del MAX98357A
Altavoz -      -> salida - del MAX98357A
```

Microfono INMP441:

```text
ESP32-S3 GPIO7  -> SCK / BCLK
ESP32-S3 GPIO15 -> WS / LRCL
ESP32-S3 GPIO16 -> SD / DOUT
ESP32-S3 3V3    -> VDD
ESP32-S3 GND    -> GND
INMP441 L/R     -> GND
```

Si conectas `L/R` del INMP441 a `3V3`, el microfono emite por el canal derecho y
habria que cambiar el codigo de `I2S_CHANNEL_FMT_ONLY_LEFT` a
`I2S_CHANNEL_FMT_ONLY_RIGHT`.
