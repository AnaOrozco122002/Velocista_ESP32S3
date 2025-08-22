// --- Sensor.cpp ---
#include "Sensor.h"
#include "Pins.h"
#include <QTRSensors16.h>

static unsigned int sensorValues[NUM_SENSORS];
volatile long lastLineRaw = 0;

static unsigned char sensorIndex[NUM_SENSORS] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

static unsigned char muxSelPins[4] = {
  (unsigned char)MUX_S0,
  (unsigned char)MUX_S1,
  (unsigned char)MUX_S2,
  (unsigned char)MUX_S3
};

static QTRSensorsMux qtra(
  sensorIndex,
  NUM_SENSORS,
  NUM_SAMPLES_PER_SENSOR,
  (unsigned char)IN_PIN,
  muxSelPins
);

float Salida = 0.0f;

void Inicializacion_Sensores() {
  for (int i = 0; i < 300; i++) {
    qtra.calibrate();
  }
}

float Lectura_Sensor() {
  unsigned long raw = qtra.readLine(sensorValues);
  lastLineRaw = (long)raw;                 // snapshot crudo
  Salida = (raw / 7500.0f) - 1.0f;         // normalizado [-1,1] aprox
  return Salida;
}

long Lectura_Linea_RAW() {
  unsigned long raw = qtra.readLine(sensorValues);
  lastLineRaw = (long)raw;
  return (long)raw;
}
