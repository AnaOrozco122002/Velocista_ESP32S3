//Incluir Librerias
#include <QTRSensors16.h>
#include <ESP32Servo.h>
#include <WiFi.h>

//Segundo Núcleo
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

//WIFI
const char *ssid = "Sollow_WIFI";
const char *password = "sofiaswifi";
byte WIFI_Status=1;
// Configuración de IP fija
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// Inicia el servidor web
WiFiServer server(80);

//Modulo de Inicio
const byte MInit = D3;
int Estado=0,Estado_L;

//TURBINA
//Creación del Objeto
Servo myTurbina;

//PIN PARA EL CONTROL DE TURBINA
const byte Tur = D4;
int ValTurb = 0,minvaltur=110,maxvaltur=180; 
float KTurb=0.7;

//Variables para sensores
#define NUM_SENSORS 16            // Numero de sensores usados
#define NUM_SAMPLES_PER_SENSOR 3  // Numero de muestras
#define IN_PIN A2                 // PIN de entrada del multiplexor


// Inicialización del sensor, digitales D9,D10,D0,D1
QTRSensorsMux qtra((unsigned char[]){ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },
                   NUM_SENSORS, NUM_SAMPLES_PER_SENSOR, (unsigned char)IN_PIN, (unsigned char[]){ D9, D10, D0, D1 });
unsigned int sensorValues[NUM_SENSORS];


//Variables para el controlador
float Tm = 4.0;  //tiempo de muestreo en mili segundos

//Variables del controlador PID
float Referencia = 0.0, Control = 0.0, Kp = 1.8, Ti = 0, Td = 0.03;
float Salida = 0.0, Error = 0.0, Error_ant = 0.0;  //variables de control
float offset = 1, Vmax = 470, E_integral;

//Variables del WIFI
String var1 = "0",var2 = "0",var3 = "0",var4 = "0",var5 = "0",var6 = "0";

//Variables Auxiliares
unsigned long int Tinicio = 0;
bool conect = false,turen = false;

//CREACIÓN DE PWM
const uint16_t Frecuencia = 13500;
const byte Canales[] = { 0, 1 };
const byte Resolucion = 10;

//Variables del Control del Motor
const int PWMI = D6;
const int PWMD = D8;
const int DirI = D5;
const int DirD = D7;



void setup() {
  Serial.begin(9600);

  //Inicialización de Segundo Nucleo
  Inicializacion_Segundo_Nucleo();

  //Inicialización de Pines
  Inicializacion_Pines();

  //Inicialización de la turbina
  Inicializacion_turbina();

  //Inicialización de Sensores
  Inicializacion_Sensores();

  //Creación del PWM
  CrearPWM();

  //Inicializacion Bluetooth
  Inicializacion_WIFI();

  //Para El funcionamiento de la Turbina
  delay(1000);

  myTurbina.write(ValTurb);
    
}

void loop() {
  portENTER_CRITICAL(&mux);
  Estado_L=Estado;
  portEXIT_CRITICAL(&mux);
  //Estado_L = digitalRead(MInit);
  //Estado_L=1; //Para Desactivar el modulo de Inicio
  while (Estado_L) {
    //Estado_L = digitalRead(MInit);
    Tinicio = millis();                         // toma el valor en milisengundos
    Salida = Lectura_Sensor();                  // funcion de lectura de la variable salida del  proceso
    Control = Controlador(Referencia, Salida);  // funcion de la ley de control
    Esfuerzo_Control(Control);                  // funcion encargada de enviar el esfuerzo de control
    //myTurbina.write(ValTurb);
    Esfuerzo_Turbina(); //Turbina Variable
    turen = true; //Variable que indica que se entro en el while
    //Serial.println("Entro");
    Tm = Tiempo_Muestreo(Tinicio);
  }
  if(turen){
    ledcWrite(Canales[0], 0);
    ledcWrite(Canales[1], 0);
    myTurbina.write(0);
    //Serial.println("no");
  }
  ledcWrite(Canales[0], 0);
  ledcWrite(Canales[1], 0);
}

//Segunda Tarea
void TaskWiFi(void *parameter) {
  for (;;) {
    if (WIFI_Status) {
      Datos();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);  // Pequeña pausa para evitar saturación
  }
}


  
//Para leer el sensor
float Lectura_Sensor(void) {
  Salida = (qtra.readLine(sensorValues) / 7500.0) - 1.0;
  //Serial.println(Salida);
  return Salida;  // retorno la variable de salidad del proceso normalizada entre 0-1, al olgoritmo de control
}

//Controlador para Motores
float Controlador(float Referencia, float Salida) {  // Funcion para la ley de control
  float E_derivativo;
  float Control;

  portENTER_CRITICAL(&mux);
  float kp_local = Kp;
  float ti_local = Ti;
  float td_local = Td;
  portEXIT_CRITICAL(&mux);
  
  Error = Referencia - Salida;
  Error = (Error > -0.2 && Error < 0) ? 0 : (Error > 0 && Error < 0.2) ? 0: Error;
  E_integral = E_integral + (((Error * (Tm / 1000.0)) + ((Tm / 1000.0) * (Error - Error_ant))) / 2.0);
  E_integral = (E_integral > 100.0) ? 100.0 : (E_integral < -100.0) ? -100: E_integral;
  E_derivativo = (Error - Error_ant) / (Tm / 1000.0);
  //Control = Kp * (Error + Ti * E_integral + Td * E_derivativo);
  Control = kp_local * (Error + ti_local * E_integral + td_local * E_derivativo);
  Error_ant = Error;
  Control = (Control > 2.5) ? 2.5 : (Control < -2.5) ? -2.5: Control;
  
  
  //Serial.println(Control);
  return Control;
}

void Esfuerzo_Control(float Control) {  //envia el esfuerzo de control en forma de PWM
  float s1, s2;

  s1 = (offset - Control);
  s2 = (offset + Control);

  ledcWrite(Canales[0], floor(constrain(abs(s1), 0.0, 1.0) * Vmax));
  ledcWrite(Canales[1], floor(constrain(abs(s2), 0.0, 1.0) * Vmax));

  if (s1 <= 0.0) {  // Motor Derecho
    digitalWrite(DirD, LOW);
  } else {
    digitalWrite(DirD, HIGH);
  }

  if (s2 <= 0.0) {  //Motor Izquierdo
    digitalWrite(DirI, LOW);
  } else {
    digitalWrite(DirI, HIGH);
  }
}

//Turbina Variable
void Esfuerzo_Turbina(){
  float estur;
  portENTER_CRITICAL(&mux);
  estur=constrain(round(minvaltur+((KTurb*abs(Error))*(maxvaltur-minvaltur))),minvaltur,maxvaltur);
  portEXIT_CRITICAL(&mux);
  //Serial.println(" Esfuerzo: " + String(estur));
  myTurbina.write(estur);
}

unsigned long int Tiempo_Muestreo(unsigned long int Tinicio) {  //, unsigned int Tm){ // Funcion que asegura que el tiempo de muestreo sea el mismo siempre
  unsigned long int T = millis() - Tinicio;
  return T;
}

void CrearPWM() {
  ledcSetup(Canales[0], Frecuencia, Resolucion);
  ledcSetup(Canales[1], Frecuencia, Resolucion);
  ledcAttachPin(PWMD, Canales[0]);
  ledcAttachPin(PWMI, Canales[1]);
}

//Inicialización del segundo núcleo
void Inicializacion_Segundo_Nucleo(){
  xTaskCreatePinnedToCore(
  TaskWiFi,        
  "TaskWiFi",      
  4096,            
  NULL,            
  1,               
  NULL,            
  1                
  );
}

void Inicializacion_turbina() {
  //Mensajes de Inicio
  ESP32PWM::allocateTimer(2);
  myTurbina.setPeriodHertz(50);       //frecuencia de la señal cuadrada
  myTurbina.attach(Tur, 1000, 2000);  //(pin,min us de pulso, máx us de pulso)
  myTurbina.write(0); 
}

void Inicializacion_Sensores() {
  //Calibración Inicial de Pines Sensor
  for (int i = 0; i < 300; i++) {  // make the calibration take about 10 seconds
    qtra.calibrate();              // reads all sensors 10 times at 2.5 ms per six sensors (i.e. ~25 ms per call)
  }
  //delay(2000);
}

void Inicializacion_Pines() {
  pinMode(PWMD, OUTPUT);
  pinMode(PWMI, OUTPUT);
  pinMode(DirI, OUTPUT);
  pinMode(DirD, OUTPUT);
  pinMode(MInit, INPUT);
  digitalWrite(DirI, LOW);
  digitalWrite(PWMI, LOW);
}

void Inicializacion_WIFI(){
  // Configuración del Access Point
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  /*Serial.println("Access Point Iniciado");
  Serial.print("IP del servidor: ");
  Serial.println(WiFi.softAPIP());*/
  server.begin();
}

void Datos(){
  WiFiClient client = server.available();
  if (client) {
    //Serial.println("Cliente conectado");
    String request = "";
    // Lee la solicitud completa del cliente
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        if (c == '\n') break;
      }
    }
    
    Serial.println("Solicitud recibida:");
    Serial.println(request);
    request.trim();  // Elimina espacios y saltos de línea


    // Extraer las 5 variables desde la URL
    if (request.indexOf("GET /?") != -1 && request.indexOf("accion=leer") == -1) {
      var1 = getValue(request, "var1=");
      var2 = getValue(request, "var2=");
      var3 = getValue(request, "var3=");
      var4 = getValue(request, "var4=");
      var5 = getValue(request, "var5=");
      var6 = getValue(request, "var6=");
      
      /*// Muestra las variables correctamente en el Serial
      Serial.println("Variables actualizadas:");
      Serial.println("Var1: " + var1);
      Serial.println("Var2: " + var2);
      Serial.println("Var3: " + var3);
      Serial.println("Var4: " + var4);
      Serial.println("Var5: " + var5);
      Serial.println("Var6: " + var6);*/

      portENTER_CRITICAL(&mux);
      if (var1 != "") Kp = var1.toFloat();
      if (var2 != "") Td = var2.toFloat();
      if (var3 != "") Ti = var3.toFloat();
      //if (var4 != "") ValTurb = var4.toFloat();
      if (var4 != "") KTurb = var4.toFloat();
      if (var5 != "") Vmax = var5.toFloat();
      if (var6 != "") offset = var6.toFloat();
      portEXIT_CRITICAL(&mux);

      /*Serial.println("Variables actualizadas:");
      Serial.println("Kp: " + String(Kp));
      Serial.println("Td " + String(Td));
      Serial.println("Ti " + String(Ti));
      Serial.println("ValTurb: " + String(ValTurb));
      Serial.println("Vmax: " + String(Vmax));
      Serial.println("offset: " + String(offset));*/
      client.println("OK");
    }

    // ✅ Responde a la app solo con los valores
    if (request.indexOf("accion=leer") != -1) {
      //Serial.println("Botón Leer presionado desde la app");
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/plain");
      client.println();
      portENTER_CRITICAL(&mux);
      client.println(String(Kp,3) + "," + String(Td,3) + "," + String(Ti) + "," + String(KTurb) + "," + String(Vmax) + "," + String(offset) + "," +  String(Estado));
      portEXIT_CRITICAL(&mux);
      client.println();
      //client.stop();
    }
    // ✅ Nueva función: Botón LEER ESFUERZO
    if (request.indexOf("accion=leer_esfuerzo") != -1) {
      //Serial.println("Botón Leer Esfuerzo presionado desde la app");
      
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println("Connection: close");
      client.println();
      client.println(",");
      portENTER_CRITICAL(&mux);
      client.println(String(Salida) + "," + String(Control) + "," + String(Error)+ "," + String(qtra.readLine(sensorValues))+ "," + String(Tm));  // ✅ Solo los valores separados por coma
      portEXIT_CRITICAL(&mux);
      client.println();
      client.stop();
    }
    if (request.indexOf("accion=inicio") != -1) {
      portENTER_CRITICAL(&mux);
      Estado = 1;
      portEXIT_CRITICAL(&mux);
      /*Serial.print("Estado: ");
      Serial.println(Estado);*/
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/plain");
      client.println();
      client.println("OK");
      client.println();
    }
    if (request.indexOf("accion=parar") != -1) {
      portENTER_CRITICAL(&mux);
      Estado = 0;
      portEXIT_CRITICAL(&mux);
      /*Serial.print("Estado: ");
      Serial.println(Estado);*/
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/plain");
      client.println();
      client.println("OK");
      client.println();
    }

    if (request == "") {
      client.println("HTTP/1.1 400 Bad Request");
      client.println("Content-type:text/plain");
      client.println();
      client.println("Error: solicitud vacía");
      client.println();
    }

    client.stop();
    //Serial.println("Cliente desconectado");
  }
}

String getValue(String data, String key) {
  int start = data.indexOf(key);
  if (start == -1) return "";
  start += key.length();
  int end = data.indexOf("&", start);
  if (end == -1) end = data.indexOf(" ", start);
  return data.substring(start, end);
}