# Robot Velocista — Versión **Doble Núcleo** (ESP32‑S3)

Este proyecto separa el **control PID y lectura de sensor** (Core 1) de la **comunicación WiFi** (Core 0) usando tareas FreeRTOS fijadas a cada núcleo. Con esto se reduce el jitter del controlador y se evita que el stack WiFi impacte la latencia del lazo.

---

## 🎯 Objetivos
- Control en tiempo (casi) real del seguidor de línea con QTR de 16 canales.
- Comunicación WiFi en modo Access Point para **ajustar parámetros** y **leer telemetría**.
- Aislar lazos críticos del stack de red usando **dos núcleos** + **secciones críticas**.

---

## 📁 Estructura de archivos
```
main.ino            // Crea tareas FreeRTOS y fija cada una a su core
Pins.h / Pins.cpp   // Pines, PWM, motores y turbina (servo)
Sensor.h / Sensor.cpp // Lectura QTR, normalización y snapshot 'lastLineRaw'
Controller.h / Controller.cpp // PID + esfuerzo de motores y turbina
WifiComms.h / WifiComms.cpp // AP + servidor HTTP, endpoints y parsing
```

---

## 🔧 Requisitos
- **Arduino IDE** 2.x
- **ESP32 core** (Espressif) ≥ 2.0.11
- Librerías:
  - `ESP32Servo`
  - `QTRSensors16` (versión 16 canales/modificada). Importar manualmente si no está en el Administrador.
- Placa: **Seeed XIAO ESP32‑S3** (o ESP32‑S3 compatible)
- Fuente de alimentación estable (servo + motores).

---

## ⚙️ Configuración rápida
En `WifiComms.cpp`:
```cpp
const char* ssid = "Sollow_WIFI";
const char* password = "sofiaswifi";
// AP: 192.168.4.1 (por defecto)
```
En `Pins.h` verifica los pines y constantes PWM/motores/servo.  
En `Pins.h` (sensores), si necesitas bajar latencia:
```cpp
#define NUM_SAMPLES_PER_SENSOR 1   // antes 3; reduce tiempo de lectura
```

---

## 🚀 Compilar y subir
1. Abre la carpeta del proyecto en Arduino IDE (todos los `.h/.cpp/.ino` juntos).
2. Selecciona la placa ESP32‑S3 correcta.
3. Compila y sube.

> Si el IDE ve dos proyectos a la vez, limpia la carpeta (evita tener dos `WifiComms.cpp` con versiones distintas).

---

## 🧠 Doble Núcleo y concurrencia
- **ControlTask** en **Core 1**: lee sensor, ejecuta PID, aplica PWM/servo.  
- **WifiTask** en **Core 0**: atiende HTTP.
- **Spinlock global** `portMUX_TYPE mux`: protege acceso a variables compartidas (Kp, Td, Ti, KTurb, Vmax, offset, Estado, Salida, Control, Error, Tm y `lastLineRaw`).  
- El sensor **solo se lee** en la tarea de control. WiFi usa el snapshot `volatile long lastLineRaw`.

---

## 🌐 Endpoints HTTP (texto plano)
- **Leer parámetros** (7 campos):  
  `GET http://192.168.4.1/?accion=leer`  
  Respuesta: `Kp,Td,Ti,KTurb,Vmax,offset,Estado`
- **Leer esfuerzo** (5 campos):  
  `GET http://192.168.4.1/?accion=leer_esfuerzo`  
  Respuesta: `Salida,Control,Error,LineaRAW,Tm`
- **Cambiar parámetros** (sin `accion=`):  
  `GET http://192.168.4.1/?var1=<Kp>&var2=<Td>&var3=<Ti>&var4=<KTurb>&var5=<Vmax>&var6=<offset>`  
  Respuesta: `OK`
- **Inicio/Parar**:  
  `GET .../?accion=inicio`  → `OK`  
  `GET .../?accion=parar`   → `OK`

> La app cliente debe **leer todo el body** (respeta `Content-Length`) y luego `split(',')`.  
> Decide por **conteo de campos**: 7 → `leer`, 5 → `leer_esfuerzo`.

---

## ⏱️ Rendimiento y timing
- `Tm` es el tiempo real del ciclo (ms). Con FreeRTOS puede subir ~1 ms sin ajustes.
- Para **periodicidad dura**:
  - Opción simple: usa `vTaskDelayUntil(..., pdMS_TO_TICKS(4))` en `ControlTask` para ~4 ms fijos.
  - Opción avanzada: **timer** con `esp_timer_start_periodic(4000)` + `ulTaskNotifyTake()` en `ControlTask`.
- Consejos:
  - Evita `Serial.print()` en `ControlTask`.
  - Mantén WiFi en Core 0 y control en Core 1.
  - Si la lectura QTR es pesada, prueba `NUM_SAMPLES_PER_SENSOR = 1`.

---

## 🧪 Depuración rápida
- Si necesitas logs de WiFi, descomenta `#define DEBUG_WIFI` en `WifiComms.cpp`.
- Si la app recibe tokens mezclados, valida que lea según `Content-Length` y haga `trim()`.

---

## 📜 Licencia
Este código se distribuye para fines educativos/experimentales. Ajusta la licencia según tus necesidades.
