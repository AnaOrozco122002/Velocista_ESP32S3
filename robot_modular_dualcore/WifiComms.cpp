// --- WifiComms.cpp (dual-core robust) ---
#include "WifiComms.h"
#include "Pins.h"
#include "Controller.h"
#include "Sensor.h"
#include <WiFi.h>

// Spinlock definido en main.ino
extern portMUX_TYPE mux;

//#define DEBUG_WIFI 1

const char* ssid = "Sollow_WIFI";
const char* password = "sofiaswifi";
byte WIFI_Status = 1;

static IPAddress local_IP(192, 168, 4, 1);
static IPAddress gateway (192, 168, 4, 1);
static IPAddress subnet  (255, 255, 255, 0);

static WiFiServer server(80);

static String var1 = "0", var2 = "0", var3 = "0", var4 = "0", var5 = "0", var6 = "0";

// --- Utilidades HTTP ---
String readHttpRequest(WiFiClient &client) {
  String req = "";
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    req += line;
    if (line == "\r") break; // fin de headers
  }
  return req;
}

void sendPlainText(WiFiClient &client, const String &body, int statusCode) {
  String status = "200 OK";
  if (statusCode == 400) status = "400 Bad Request";
  else if (statusCode == 404) status = "404 Not Found";

  client.print("HTTP/1.1 " + status + "\r\n");
  client.print("Content-Type: text/plain\r\n");
  client.print("Connection: close\r\n");
  client.print("Content-Length: ");
  client.print(body.length());
  client.print("\r\n\r\n");
  client.print(body);
  client.print("\r\n");
  client.flush();
  client.stop();
}

// --- Inicialización AP ---
void Inicializacion_WIFI() {
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  server.begin();
#ifdef DEBUG_WIFI
  Serial.println("[WiFi] AP iniciado en 192.168.4.1");
#endif
}

// --- Handler principal ---
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

  // 1) leer_esfuerzo (5 campos): Salida, Control, Error, LineaRAW, Tm
  if (request.indexOf("accion=leer_esfuerzo") != -1) {
    float s, c, e;
    long raw;
    unsigned long tm;

    portENTER_CRITICAL(&mux);
    s   = Salida;
    c   = Control;
    e   = Error;
    raw = lastLineRaw;
    tm  = (unsigned long)Tm;
    portEXIT_CRITICAL(&mux);

    String body = String(s, 3) + "," + String(c, 3) + "," + String(e, 4) + "," +
                  String(raw) + "," + String(tm);
#ifdef DEBUG_WIFI
    Serial.println("[WiFi] leer_esfuerzo -> " + body);
#endif
    sendPlainText(client, body, 200);
    return;
  }

  // 2) leer (7 campos): Kp,Td,Ti,KTurb,Vmax,offset,Estado
  if (request.indexOf("accion=leer") != -1) {
    float kp, td, ti, kt, vmax, off;
    int   est;

    portENTER_CRITICAL(&mux);
    kp = Kp; td = Td; ti = Ti; kt = KTurb; vmax = Vmax; off = offset; est = Estado;
    portEXIT_CRITICAL(&mux);

    String body = String(kp, 2) + "," + String(td, 4) + "," + String(ti, 2) + "," +
                  String(kt, 2) + "," + String(vmax, 1) + "," + String(off, 1) + "," +
                  String(est);
#ifdef DEBUG_WIFI
    Serial.println("[WiFi] leer -> " + body);
#endif
    sendPlainText(client, body, 200);
    return;
  }

  // 3) inicio / parar
  if (request.indexOf("accion=inicio") != -1) {
    portENTER_CRITICAL(&mux);
    Estado = 1;
    portEXIT_CRITICAL(&mux);
    sendPlainText(client, "OK", 200);
    return;
  }
  if (request.indexOf("accion=parar") != -1) {
    portENTER_CRITICAL(&mux);
    Estado = 0;
    portEXIT_CRITICAL(&mux);
    sendPlainText(client, "OK", 200);
    return;
  }

  // 4) Actualización de parámetros (sin accion=*)
  if (request.indexOf("GET /?") != -1 &&
      request.indexOf("accion=") == -1) {

    String v1 = getValue(request, "var1=");
    String v2 = getValue(request, "var2=");
    String v3 = getValue(request, "var3=");
    String v4 = getValue(request, "var4=");
    String v5 = getValue(request, "var5=");
    String v6 = getValue(request, "var6=");

    portENTER_CRITICAL(&mux);
    if (v1.length()) Kp     = v1.toFloat();
    if (v2.length()) Td     = v2.toFloat();
    if (v3.length()) Ti     = v3.toFloat();
    if (v4.length()) KTurb  = v4.toFloat();
    if (v5.length()) Vmax   = v5.toFloat();
    if (v6.length()) offset = v6.toFloat();
    portEXIT_CRITICAL(&mux);

    sendPlainText(client, "OK", 200);
    return;
  }

  // 5) Default
  sendPlainText(client, "Not Found", 404);
}

String getValue(const String& data, const String& key) {
  int start = data.indexOf(key);
  if (start == -1) return "";
  start += key.length();
  int end = data.indexOf("&", start);
  if (end == -1) end = data.indexOf(" ", start);
  if (end == -1) end = data.length();
  return data.substring(start, end);
}
