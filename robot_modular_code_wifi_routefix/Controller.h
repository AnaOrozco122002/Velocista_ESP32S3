// --- Controller.h ---
#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

extern Servo myTurbina;

extern float Tm;
extern float Referencia;
extern float Control;
extern float Kp, Ti, Td;
extern float Error;
extern float offset, Vmax, E_integral;
extern float KTurb;

void Inicializacion_turbina();
float Controlador(float Ref, float Salida);
void  Esfuerzo_Control(float u);
void  Esfuerzo_Turbina();
unsigned long Tiempo_Muestreo(unsigned long Tinicio);
