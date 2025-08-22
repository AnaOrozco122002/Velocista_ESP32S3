// --- Pins.h ---
#pragma once
#include <Arduino.h>

extern const byte MInit;
extern int Estado;

extern const byte Tur;
extern int ValTurb;
extern int minvaltur;
extern int maxvaltur;

extern const uint16_t Frecuencia;
extern const byte Canales[2];
extern const byte Resolucion;

extern const int PWMI;
extern const int PWMD;
extern const int DirI;
extern const int DirD;

#define NUM_SENSORS 16
#define NUM_SAMPLES_PER_SENSOR 3
extern const int IN_PIN;
extern const byte MUX_S0;
extern const byte MUX_S1;
extern const byte MUX_S2;
extern const byte MUX_S3;

void Inicializacion_Pines();
void CrearPWM();
