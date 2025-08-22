// --- WifiComms.cpp (ROBUST + sanitizado) ---
#include "WifiComms.h"
#include "Pins.h"
#include "Controller.h"
#include "Sensor.h"
#include <WiFi.h>
#include <math.h>

//#define DEBUG_WIFI 1

const char* ssid = "Sollow_WIFI";
const char* password = "sofiaswifi";
byte WIFI_Status = 1;

static IPAddress local_IP(192, 168, 4, 1);
static IPAddress gateway (192, 168, 4, 1);
static IPAddress subnet  (255, 255, 255, 0);

static WiFiServer server(80);

static String var1 = "0", var2 = "0", var3 = "0", var4 = "0", var5 = "0", var6 = "0";

// ---------- Utilidades ----------

// Lee hasta fin de cabeceras: \r\n\r\n
String readHttpRequest(WiFiClient &client) {
  String req = "";
  while (client.connected()) {
    String line = client.readStringUntil('\n'); // incluye \r cuando llega
    req += line;
    if (line == "\r") break; // línea en blanco: fin de headers
  }
  return req;
}

// Filtro simple: solo deja números, signo, punto y notación científica
String filterNumeric(const String &s) {
  String out = "";
  for (size_t i = 0; i < s.length(); ++i) {
    char c = s[i];
    if ((c >= '0' && c <= '9') || c=='-' || c=='+' || c=='.' || c=='e' || c=='E') {
      out += c;
    }
  }
  return out;
}

String f2s(float v, unsigned int digits = 6) {
  if (!isfinite(v)) return "0";
  return String(v, (unsigned int)digits); // SIEMPRE con punto decimal
}
String l2s(long v) { return String(v); }
String u2s(unsigned long v) { return String(v); }

// Envía respuesta text/plain con Content-Length correcto
void sendPlainText(WiFiClient &client, const String &body, int statusCode) {
  String status = "200 OK";
  if (statusCode == 400) status = "400 Bad Request";
  else if (statusCode == 404) status = "404 Not Found";

  client.print("HTTP/1.1 " + status + "\r\n");
  client.print("Content-Type: text/plain\r\n");
  client.print("Connection: close\r\n");
  client.print("Content-Length: ");
  client.print(body.length());
  client.print("\r\n\r\n"); // fin de headers

  client.print(body);
  client.print("\r\n"); // opcional
  client.flush();
  client.stop();
}

// ---------- Inicialización AP ----------
void Inicializacion_WIFI() {
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  server.begin();
#ifdef DEBUG_WIFI
  Serial.println("[WiFi] AP iniciado en 192.168.4.1");
#endif
}

// ---------- Handler principal ----------
void Datos() {
  WiFiClient client = server.available();
  if (!client) return;

  client.setTimeout(2000);
  client.setNoDelay(true);

  String request = readHttpRequest(client);
#ifdef DEBUG_WIFI
  Serial.println("---- HTTP REQUEST ----");
  Serial.println(request);
  Serial.println("----------------------");
#endif
  if (request.length() == 0) {
    sendPlainText(client, "Bad Request", 400);
    return;
  }

  // 1) leer_esfuerzo (antes que 'leer' para evitar coincidencia parcial)
  if (request.indexOf("accion=leer_esfuerzo") != -1) {
    String body = f2s(Salida, 4) + "," + f2s(Control, 4) + "," + f2s(Error, 4) + "," +
                  l2s((long)Lectura_Linea_RAW()) + "," + u2s((unsigned long)Tm);
#ifdef DEBUG_WIFI
    Serial.println("[WiFi] leer_esfuerzo -> " + body);
#endif
    sendPlainText(client, body, 200);
    return;
  }

  // 2) leer (parámetros actuales)
  if (request.indexOf("accion=leer") != -1) {
    // Limpieza de números y formato homogéneo
    String body = f2s(Kp, 2)    + "," + f2s(Td, 4)    + "," + f2s(Ti, 2) + "," +
                  f2s(KTurb, 2) + "," + f2s(Vmax, 1)  + "," + f2s(offset, 1) + "," +
                  String(Estado);
#ifdef DEBUG_WIFI
    Serial.println("[WiFi] leer -> " + body);
#endif
    sendPlainText(client, body, 200);
    return;
  }

  // 3) inicio/parar
  if (request.indexOf("accion=inicio") != -1) {
    Estado = 1;
#ifdef DEBUG_WIFI
    Serial.println("[WiFi] accion=inicio -> Estado=1");
#endif
    sendPlainText(client, "OK", 200);
    return;
  }
  if (request.indexOf("accion=parar") != -1) {
    Estado = 0;
#ifdef DEBUG_WIFI
    Serial.println("[WiFi] accion=parar -> Estado=0");
#endif
    sendPlainText(client, "OK", 200);
    return;
  }

  // 4) Actualización de parámetros por GET /?var1=... (sin accion=*)
  if (request.indexOf("GET /?") != -1 &&
      request.indexOf("accion=") == -1) {

    var1 = filterNumeric(getValue(request, "var1=")); var1.trim();
    var2 = filterNumeric(getValue(request, "var2=")); var2.trim();
    var3 = filterNumeric(getValue(request, "var3=")); var3.trim();
    var4 = filterNumeric(getValue(request, "var4=")); var4.trim();
    var5 = filterNumeric(getValue(request, "var5=")); var5.trim();
    var6 = filterNumeric(getValue(request, "var6=")); var6.trim();

    if (var1.length()) Kp     = var1.toFloat();
    if (var2.length()) Td     = var2.toFloat();
    if (var3.length()) Ti     = var3.toFloat();
    if (var4.length()) KTurb  = var4.toFloat();
    if (var5.length()) Vmax   = var5.toFloat();
    if (var6.length()) offset = var6.toFloat();

#ifdef DEBUG_WIFI
    Serial.println("[WiFi] Vars -> Kp=" + String(Kp,6) + " Td=" + String(Td,6) +
                   " Ti=" + String(Ti,6) + " KTurb=" + String(KTurb,6) +
                   " Vmax=" + String(Vmax,3) + " offset=" + String(offset,3));
#endif
    sendPlainText(client, "OK", 200);
    return;
  }

  // 5) Default
  sendPlainText(client, "Not Found", 404);
}

// ---------- Utilidad de parsing simple ----------
String getValue(const String& data, const String& key) {
  int start = data.indexOf(key);
  if (start == -1) return "";
  start += key.length();
  int end = data.indexOf("&", start);
  if (end == -1) end = data.indexOf(" ", start);
  if (end == -1) end = data.length();
  return data.substring(start, end);
}
