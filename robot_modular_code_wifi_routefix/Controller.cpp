// --- Controller.cpp ---
#include "Controller.h"
#include "Pins.h"

float Tm = 4.0f;
float Referencia = 0.0f, Control = 0.0f;
float Kp = 1.8f, Ti = 0.0f, Td = 0.03f;
float Error = 0.0f, E_integral = 0.0f, offset = 1.0f, Vmax = 470.0f;
static float Error_ant = 0.0f;
float KTurb = 0.7f;

Servo myTurbina;

void Inicializacion_turbina() {
  ESP32PWM::allocateTimer(2);
  myTurbina.setPeriodHertz(50);
  myTurbina.attach(Tur, 1000, 2000);
  myTurbina.write(0);
}

float Controlador(float Ref, float Sal) {
  float E_derivativo;
  float u;

  Error = Ref - Sal;
  if ((Error > -0.2f && Error < 0.0f) || (Error > 0.0f && Error < 0.2f)) Error = 0.0f;

  E_integral = E_integral + (((Error * (Tm / 1000.0f)) + ((Tm / 1000.0f) * (Error - Error_ant))) / 2.0f);
  E_integral  = constrain(E_integral, -100.0f, 100.0f);

  E_derivativo = (Error - Error_ant) / (Tm / 1000.0f);
  u = Kp * (Error + Ti * E_integral + Td * E_derivativo);
  Error_ant = Error;

  if (u > 2.5f) u = 2.5f;
  if (u < -2.5f) u = -2.5f;

  return u;
}

void Esfuerzo_Control(float u) {
  float s1 = (offset - u);
  float s2 = (offset + u);

  ledcWrite(Canales[0], floor(constrain(fabs(s1), 0.0f, 1.0f) * Vmax));
  ledcWrite(Canales[1], floor(constrain(fabs(s2), 0.0f, 1.0f) * Vmax));

  digitalWrite(DirD, (s1 <= 0.0f) ? LOW  : HIGH);
  digitalWrite(DirI, (s2 <= 0.0f) ? LOW  : HIGH);
}

void Esfuerzo_Turbina() {
  float estur = constrain(round(minvaltur + ((KTurb * fabs(Error)) * (maxvaltur - minvaltur))), (float)minvaltur, (float)maxvaltur);
  myTurbina.write((int)estur);
}

unsigned long Tiempo_Muestreo(unsigned long Tinicio) {
  return millis() - Tinicio;
}
