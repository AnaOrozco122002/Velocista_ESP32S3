// --- WifiComms.h ---
#pragma once
#include <Arduino.h>

extern const char* ssid;
extern const char* password;
extern byte WIFI_Status;

void Inicializacion_WIFI();
void Datos();
String getValue(const String& data, const String& key);
