# Robot Velocista — Versión **Un Solo Núcleo** (ESP32)

Esta variante ejecuta **control + WiFi** en el mismo núcleo (estilo Arduino clásico). Es más simple y suficiente si la carga de red es baja y el lazo tolera un poco más de jitter.

---

## 🎯 Objetivos
- Control PID + lectura QTR en el loop principal.
- WiFi AP para ajustar parámetros y leer telemetría.
- Arquitectura modular en archivos `.h/.cpp` para mantenimiento.

---

## 📁 Estructura de archivos
```
main.ino            // setup/loop tradicionales
Pins.h / Pins.cpp   // Pines, PWM, motores y turbina
Sensor.h / Sensor.cpp // Lectura QTR y normalización
Controller.h / Controller.cpp // PID + esfuerzos
WifiComms.h / WifiComms.cpp // AP + servidor HTTP, endpoints y parsing
```

---

## 🔧 Requisitos
- **Arduino IDE** 2.x
- **ESP32 core** (Espressif) ≥ 2.0.11
- Librerías:
  - `ESP32Servo`
  - `QTRSensors16` (16 canales/modificada). Importar manualmente si no está en el Administrador.
- Placa ESP32 compatible (ESP32‑S3 también funciona en modo single‑core).

---

## ⚙️ Configuración rápida
En `WifiComms.cpp`:
```cpp
const char* ssid = "Sollow_WIFI";
const char* password = "sofiaswifi";
// AP: 192.168.4.1
```
Ajusta pines/constantes en `Pins.h`.  
Si necesitas menor latencia de lectura del sensor, prueba:
```cpp
#define NUM_SAMPLES_PER_SENSOR 1
```

---

## 🚀 Compilar y subir
1. Abre el proyecto en Arduino IDE (todos los `.h/.cpp/.ino` en la misma carpeta).
2. Selecciona la placa ESP32 correcta.
3. Compila y sube.

> Si mezclas versiones, evita tener duplicados de `.cpp` con el mismo nombre.

---

## 🌐 Endpoints HTTP (texto plano)
- **Leer parámetros** (7 campos):  
  `GET http://192.168.4.1/?accion=leer`  
  `Kp,Td,Ti,KTurb,Vmax,offset,Estado`
- **Leer esfuerzo** (5 campos):  
  `GET http://192.168.4.1/?accion=leer_esfuerzo`  
  `Salida,Control,Error,LineaRAW,Tm`
- **Cambiar parámetros**:  
  `GET .../?var1=<Kp>&var2=<Td>&var3=<Ti>&var4=<KTurb>&var5=<Vmax>&var6=<offset>` → `OK`
- **Inicio/Parar**:  
  `GET .../?accion=inicio` / `?accion=parar` → `OK`

> Cliente: lee todo el body, `trim()` y `split(',')`. 7 campos → `leer`, 5 campos → `leer_esfuerzo`.

---

## ⏱️ Notas de rendimiento
- Con un solo núcleo, el WiFi puede introducir **jitter** en el lazo si hay muchas solicitudes.
- Recomendaciones:
  - Evita `Serial.print()` dentro del control.
  - Mantén `WifiComms::Datos()` **no bloqueante** (tal como está).
  - Si el lazo es crítico, considera migrar a la versión **doble núcleo**.

---

## 🧪 Depuración
- Para ver solicitudes/respuestas, puedes agregar prints en `WifiComms.cpp` (recuerda que afectan al timing).
- Si la app “mezcla” tokens, revisa `Content-Length` y que haces `split(',')` tras `trim()`.

---

## 📜 Licencia
Este código se distribuye para fines educativos/experimentales. Ajusta la licencia según tus necesidades.
