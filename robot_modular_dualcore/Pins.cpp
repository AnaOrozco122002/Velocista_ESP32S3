// --- Pins.cpp ---
#include "Pins.h"

const byte MInit = D3;
int Estado = 0;

const byte Tur = D4;
int ValTurb = 0;
int minvaltur = 110;
int maxvaltur = 180;

const uint16_t Frecuencia = 13500;
const byte Canales[2] = {0, 1};
const byte Resolucion = 10;

const int PWMI = D6;
const int PWMD = D8;
const int DirI = D5;
const int DirD = D7;

const int IN_PIN = A2;
const byte MUX_S0 = D9;
const byte MUX_S1 = D10;
const byte MUX_S2 = D0;
const byte MUX_S3 = D1;

void Inicializacion_Pines() {
  pinMode(PWMD, OUTPUT);
  pinMode(PWMI, OUTPUT);
  pinMode(DirI, OUTPUT);
  pinMode(DirD, OUTPUT);
  pinMode(MInit, INPUT);

  digitalWrite(DirI, LOW);
  digitalWrite(PWMI, LOW);
}

void CrearPWM() {
  ledcSetup(Canales[0], Frecuencia, Resolucion);
  ledcSetup(Canales[1], Frecuencia, Resolucion);
  ledcAttachPin(PWMD, Canales[0]);
  ledcAttachPin(PWMI, Canales[1]);
}
