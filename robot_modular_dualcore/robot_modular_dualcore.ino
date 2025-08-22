// --- main.ino (dual-core) ---
#include "Pins.h"
#include "Sensor.h"
#include "Controller.h"
#include "WifiComms.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Spinlock global compartido
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// Handles de las tareas
TaskHandle_t controlTaskHandle = nullptr;
TaskHandle_t wifiTaskHandle    = nullptr;

// Bucle de control (Core 1)
void ControlTask(void* pv) {
  for (;;) {
    if (Estado) {
      unsigned long Tinicio = millis();

      float salidaLocal = Lectura_Sensor();
      float uLocal      = Controlador(Referencia, salidaLocal);

      // actualizar Control global de forma segura
      portENTER_CRITICAL(&mux);
      Control = uLocal;
      portEXIT_CRITICAL(&mux);

      Esfuerzo_Control(uLocal);
      Esfuerzo_Turbina();

      // Actualizar Tm real
      Tm = Tiempo_Muestreo(Tinicio);
    } else {
      // STOP motores y turbina
      ledcWrite(Canales[0], 0);
      ledcWrite(Canales[1], 0);
      myTurbina.write(0);
      vTaskDelay(pdMS_TO_TICKS(1));
    }
    taskYIELD();
  }
}

// Bucle WiFi (Core 0)
void WifiTask(void* pv) {
  for (;;) {
    if (WIFI_Status) {
      Datos();
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void setup() {
  Serial.begin(9600);

  Inicializacion_Pines();
  Inicializacion_turbina();
  Inicializacion_Sensores();
  CrearPWM();
  Inicializacion_WIFI();

  delay(1000);
  myTurbina.write(ValTurb);

  // Control en Core 1 (APP), WiFi en Core 0 (PRO)
  xTaskCreatePinnedToCore(ControlTask, "ControlTask", 4096, nullptr, 3, &controlTaskHandle, 1);
  xTaskCreatePinnedToCore(WifiTask,    "WifiTask",    6144,  nullptr, 1, &wifiTaskHandle,    0);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
