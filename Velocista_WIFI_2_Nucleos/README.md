# 🤖 Robot Seguidor con Control PID y Comunicación WiFi (ESP32-S3)

Este proyecto implementa un sistema de control para un robot seguidor de línea utilizando sensores QTR, control PID, y una turbina de asistencia. Está diseñado para ejecutarse en una placa **XIAO ESP32-S3**, aprovechando sus **dos núcleos**: uno para el control del robot y otro para la comunicación inalámbrica vía WiFi en modo Access Point.

---

## ⚙️ Características principales

- 🧠 Control PID ajustable en tiempo real vía red WiFi.
- 📶 Comunicación WiFi mediante servidor HTTP (modo AP).
- 🧲 Lectura de sensores QTR (16 sensores, multiplexado).
- 🌪️ Control de turbina proporcional al error.
- 🔄 Control de motores con PWM de alta frecuencia.
- 🧵 Ejecución multihilo usando los dos núcleos del ESP32-S3 (FreeRTOS).

---

## 📦 Estructura del proyecto

- `setup()` inicializa todos los componentes (PWM, sensores, WiFi, etc).
- `loop()` ejecuta el control de línea y turbina.
- `TaskWiFi()` corre en el **segundo núcleo** para manejar solicitudes WiFi.
- `Datos()` procesa comandos desde una app/web vía HTTP GET.
- `Controlador()` implementa la ley PID.
- Se utilizan **zonas críticas** (`portENTER_CRITICAL`) para proteger variables compartidas entre núcleos.

---

## 🔌 Requisitos de hardware

- 📟 Placa: **Seeed Studio XIAO ESP32-S3**
- 🔋 Motores DC con control PWM
- ⚙️ Turbina (servo)
- 🔦 Sensor QTR-16 (o QTRX-MD-16A)
- 🔌 Pines conectados a:
  - PWM motores: `D6`, `D8`
  - Direcciones motores: `D5`, `D7`
  - Turbina: `D4`
  - Multiplexor sensores: `D9`, `D10`, `D0`, `D1`
  - Entrada sensores: `A2`
  - Botón inicio: `D3`

---

## 🧰 Librerías necesarias

Instala desde el Library Manager de Arduino IDE:

- `QTRSensors16`
- `ESP32Servo`
- `WiFi.h`

---

## 📶 Comunicación WiFi

El ESP32 crea una red WiFi con:

- **SSID**: `Sollow_WIFI`
- **Contraseña**: `sofiaswifi`
- **IP fija**: `192.168.4.1`
