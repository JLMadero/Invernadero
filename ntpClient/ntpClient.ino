#include <NoDelay.h>
#include <WiFi.h>
#include "time.h"
// Substituya "SSID" por el nombre del SSID de la red Wifi a
// conectarse
const char* ssid = "SSID";

// Substituya "contraseña" por la contraseña de la red Wifi a
// conectarse
const char* password = "contraseña";
// Velocidad de transmisión del puerto serie
const unsigned int BAUD_RATE = 115200;
// URL de un servidor NTP (Network Time Protocol)
const char* ntpServer = "pool.ntp.org";
// Offset en segundos de la zona horaria local con respecto a GMT
const long gmtOffset_sec = -25200;  // -7*3600
// Offset en segundos del tiempo, en el caso de horario de verano
const int daylightOffset_sec = 0;
// Estructura con la informacion de la fecha/hora actual
struct tm timeinfo;
void conectaRedWifi(const char* ssid, const char* password);
void actualizaLectura();
void printLocalTime(struct tm* ptimeinfo);
void printLocalTime2(struct tm* ptimeinfo);
void printLocalTime3(struct tm* ptimeinfo);
// Tiempo entre despliegues de la fecha/hora
const long PAUSA = 1000;
// Crea una instancia de la clase noDelay
// que determina si han transcurrido PERIODO ms
noDelay pausa(PAUSA);
void setup() {
  // Inicializa la comunicación serial
  Serial.begin(BAUD_RATE);
  delay(100);

  // Conecta a una red Wifi
  conectaRedWiFi(ssid, password);

  // Inicializa el cliente NTP, obtiene la fecha/hora del
  // servidor NTP e inicializa el reloj interno del
  // microcontrolador ESP32
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  // Verifica si es tiempo de actualizar la fecha/hora
  if (pausa.update()) {
    // Obtiene del reloj interno del microcontrolador la fecha
    // y hora y las despliega en el monitor serial
    actualizaLectura();
  }
}
/*
 * Conecta el microcontrolador ESP32 a una red WiFi
 */
void conectaRedWiFi(const char* ssid, const char* password) {
  // Conexion a la red
  WiFi.begin(ssid, password);
  Serial.print("Conectandose a la red ");
  Serial.print(ssid);
  Serial.println(" ...");

  // Mientras no se ha conectado a la red WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println('\n');
  Serial.println("Connexion establecida");

  // Obten la direccion IP del microcontrolador ESP32
  Serial.print("Direccion IP del servidor web: ");
  Serial.println(WiFi.localIP());
}
/**
 * Esta funcion obtiene del reloj interno del microcontrolador
 * la fecha y hora y las despliega en el monitor serial
 */
void actualizaLectura() {
  // Obtiene la fecha y hora
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No se pudo obtener la fecha/hora");
  }
  // Despliega la fecha/hora en el monitor serial accediendo
  // directamente a la estructura struct tm
  printLocalTime(&timeinfo);
  // Despliega la fecha/hora en el monitor serial formateando
  // la fecha usando la funcion strftime()
  printLocalTime2(&timeinfo);
  // Despliega la fecha/hora en el monitor serial formateando
  // la fecha usando la funcion Serial.print()/Serial.println()
  printLocalTime3(&timeinfo);
}
/**
 * Esta función despliega la fecha y hora actual
 * accesando directamente

 */
void printLocalTime(struct tm* ptimeinfo) {
  Serial.print("Fecha en formato dd/mm/aa hh:mm:ss: ");
  Serial.print(ptimeinfo->tm_mday);
  Serial.print("/");
  Serial.print(ptimeinfo->tm_mon + 1);
  Serial.print("/");
  Serial.print(ptimeinfo->tm_year + 1900);
  Serial.print(" ");
  Serial.print(ptimeinfo->tm_hour);
  Serial.print(":");
  Serial.print(ptimeinfo->tm_min);
  Serial.print(":");
  Serial.println(ptimeinfo->tm_sec);
}
/**
 * Esta función despliega la fecha y hora actual
 * formateada usando la funcion strftime()
 */
void printLocalTime2(struct tm* ptimeinfo) {
  char sfecha[80];

  strftime(sfecha, 80, "%A, %d %B %Y %H:%M:%S", ptimeinfo);
  Serial.print("Fecha: ");
  Serial.println(sfecha);
}
/**
 * Esta función despliega la fecha y hora actual
 * formateada usando la funcion Serial.print()
 * o Serial.println()
 */
void printLocalTime3(struct tm* ptimeinfo) {
  Serial.print("Fecha en formato mm/dd/aa hh:mm:ss: ");
  Serial.println(ptimeinfo, "%x, %X");
}
