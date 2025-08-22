// --- main.ino ---
#include "Pins.h"
#include "Sensor.h"
#include "Controller.h"
#include "WifiComms.h"

unsigned long Tinicio = 0;
bool turen = false;

void setup() {
  Serial.begin(9600);

  Inicializacion_Pines();
  Inicializacion_turbina();
  Inicializacion_Sensores();
  CrearPWM();
  Inicializacion_WIFI();

  delay(1000);
  myTurbina.write(ValTurb);
}

void loop() {
  //Estado=1;
  //Estado = digitalRead(MInit);
  while (Estado) {
    //Estado = digitalRead(MInit);
    Tinicio = millis();
    Salida  = Lectura_Sensor();
    Control = Controlador(Referencia, Salida);
    Esfuerzo_Control(Control);
    Esfuerzo_Turbina();

    if (WIFI_Status) {
      Datos();
    }

    turen = true;
    Tm = Tiempo_Muestreo(Tinicio);
  }

  if (turen) {
    ledcWrite(Canales[0], 0);
    ledcWrite(Canales[1], 0);
    myTurbina.write(0);
    if (WIFI_Status) Datos();
  }

  ledcWrite(Canales[0], 0);
  ledcWrite(Canales[1], 0);
  if (WIFI_Status) Datos();
}
