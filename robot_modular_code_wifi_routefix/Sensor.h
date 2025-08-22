// --- Sensor.h ---
#pragma once
#include <Arduino.h>

void Inicializacion_Sensores();
float Lectura_Sensor();
long  Lectura_Linea_RAW();

extern float Salida;
