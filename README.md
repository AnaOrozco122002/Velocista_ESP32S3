# 🚀 Código del Velocista *Sollow*

### 🤖 Proyecto desarrollado para **Semillero de Robótica**  
📍 *Universidad del Cauca*

---

## 📜 Descripción General

Este repositorio contiene el código para un robot velocista con **turbina y control inalámbrico**, desarrollado usando una placa **XIAO ESP32S3**.  
Fue escrito en el **IDE de Arduino (versión 2.3.5)**, utilizando la plataforma **ESP32 (versión 2.0.11)**.

🔗 Para configurar tu IDE con placas ESP32, agrega el siguiente enlace al Gestor de URLs adicionales:  
```txt
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

---

## 📦 Librerías y Archivos Adicionales

- ✅ Se usa la librería **QTRSensors16**, una versión personalizada para sensores de 16 canales.
  - Asegúrate de **importarla manualmente** desde la carpeta raíz del proyecto.
  - Instrucciones de importación:
  
    ![](https://github.com/AnaOrozco122002/Velocista/blob/master/images/add.jpg)

---

## 🔌 Mapeo de Pines

| Función                        | Pin              |
|-------------------------------|------------------|
| Sensor analógico              | A2               |
| Lectura del multiplexor       | D0, D1, D9, D10  |
| PWM - Motor Derecho           | D8               |
| PWM - Motor Izquierdo         | D6               |
| Dirección - Motor Derecho     | D7               |
| Dirección - Motor Izquierdo   | D5               |
| Control de Turbina            | D4               |
| Módulo de Inicio              | D3               |

---

## 🧩 Componentes Principales

### 📍 Microcontrolador: **XIAO ESP32S3**

![](https://github.com/AnaOrozco122002/Velocista/blob/master/images/xiaopines.jpg)

Placa compacta, potente y eficiente, ideal para proyectos con BLE y espacio reducido.

---

### 🔧 Drivers: **2x DRV LFX9201**

![](https://github.com/AnaOrozco122002/Velocista/blob/master/images/drvpin.jpg)

Drivers eficientes para control independiente de motores DC.

---

### ⚡ Regulador de Voltaje

![](https://github.com/AnaOrozco122002/Velocista/blob/master/images/regul.jpg)

Convierte los **11.9V / 1500mA** de la batería a los 5V necesarios para los módulos electrónicos.

---

### 📏 Sensor de Línea: **QTR-16 XL**

![](https://github.com/AnaOrozco122002/Velocista/blob/master/images/sensor.jpg)

Sensor infrarrojo de alta resolución con 16 canales, ideal para seguimiento de línea en alta velocidad.

---

## 🛠️ Créditos

Desarrollado por integrantes del **Semillero de Robótica - Universidad del Cauca**.  
Para más información o colaboración, contacta al equipo a través del repositorio o canales institucionales.
