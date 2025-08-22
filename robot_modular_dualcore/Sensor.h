// --- Sensor.h ---
#pragma once
#include <Arduino.h>

void Inicializacion_Sensores();
float Lectura_Sensor();     // Actualiza Salida y lastLineRaw
long  Lectura_Linea_RAW();  // Lectura directa (si se requiere)

extern float Salida;
extern volatile long lastLineRaw;   // Snapshot crudo accesible para WiFi
