#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <LittleFS.h>
#include "time.h"
#include "DHT.h"
#include <NoDelay.h>
// Substituya "SSID" por el nombre del SSID de la red Wifi a
// conectarse
const char* ssid = "SSID";

// Substituya "contraseña" por la contraseña de la red Wifi a
// conectarse
const char* password = "contraseña";
// Pin a la que esta conectado el sensor DHT11, GPIO4
const unsigned int PIN_DHT = 4;
// Velocidad de transmision del puerto serie
const unsigned int BAUD_RATE = 115200;
// URL de un servidor NTP (Network Time Protocol)
const char* ntpServer = "pool.ntp.org";
// Offset en segundos de la zona horaria local con respecto a GMT
const long gmtOffset_sec = -25200;  // -7*3600
// Offset en segundos del tiempo, en el caso de horario de verano
const int daylightOffset_sec = 0;
// Tiempo entre lecturas del sensor DHT11
const long PAUSA = 2000;
// Estructura con la informacion de la fecha/hora actual
struct tm timeinfo;
// Crea una instancia de la clase DHT11
DHT dht(PIN_DHT, DHT11);
// Crea una instancia de un servidor web que
// escucha en el puerto 80
AsyncWebServer server(80);
// Crea una instancia de la clase noDelay
// que determina si han transcurrido PERIODO ms
noDelay pausa(PAUSA);
// Variable con la fecha y hora del sistema
String fecha;
// Variable con el valor de la humedad
String humedad;
// Variable con el valor de la temperatura en grados centígrados
String temperaturaCent;
// Variable con el valor de la temperatura en grados Fahrenheith
String temperaturaFahr;
// Variable con el valor del indice de calor en grados centígrados
String indiceCalorCent;
// Variable con el valor del indice de calor en grados Fahrenheith
String indiceCalorFahr;
void conectaRedWifi(const char* ssid, const char* password);
void inicializaLittleFS();
void configuraServidor();
void noHallada(AsyncWebServerRequest* request);
void actualizaLectura();
String obtenFecha();
String obtenHumedad();
String obtenTempCent();
String obtenTempFahr();
String obtenIndiceCalorCent();
String obtenIndiceCalorFahr();
String processor(const String& var);
void setup() {
  // Inicializa la comunicacion serial
  Serial.begin(BAUD_RATE);
  delay(100);

  // Inicializa el sensor DHT11
  dht.begin();
  // Conecta a una red Wifi
  conectaRedWiFi(ssid, password);

  // Monta el sistema de archivos LittleFS
  inicializaLittleFS();

  // Inicializa el cliente NTP, obtiene la fecha/hora del servidor
  // NTP e inicializa el reloj interno del microcontrolador ESP32
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // Mapea las diferentes funcionalidades del servidor a los URL
  // con las que seran invocadas
  configuraServidor();
  // Inicializa el servidor web
  server.begin();
  Serial.println("Servidor web inicializado");
}
void loop() {
  // Verifica si es tiempo de leer el reloj interno del
  // microcontrolador y el sensor DHT11
  if (pausa.update()) {
    // Lee el reloj interno del microcontrolador
    // y el sensor DHT11
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
 * Esta funcion monta el sistema de archivos LittleFS
 */
void inicializaLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("Ocurrió un error al montar LittleFS");
  } else {
    Serial.println("Se monto LittleFS con exito");
  }
}
/*
 * Mapea las diferentes funcionalidades del servidor a los URL
 * con las que seran invocadas
 */
void configuraServidor() {
  // Carga los archivos estaticos desde la raiz del sistema de
  // archivos LittleFS
  server.serveStatic("/", LittleFS, "/");
  // Si se invoca al servidor con la URL:
  // "direccionIP_ServidorWev/"
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Le envia al cliente el mensaje de respuesta con la pagina
    // de inicio: index.html. Recibe como argumentos:
    // - LittleFS: Indica que los archivos a servir se
    // encuentran en el sistema de archivos LittleFS
    // - index.html, Nombre del archivo a enviar
    // - "text/html": El tipo del contenido del archivo:
    // texto plano, codigo HTML.
    request->send(LittleFS, "/index.html", "text/html");
  });
  // Si se invoca al servidor con la URL:
  // "direccionIP_ServidorWev/a"
  // O si se presiona el el boton "Recargar"
  // en la pagina temperatura.html
  server.on("/a", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Le envia al cliente el mensaje de respuesta con la pagina
    // con la fecha y hora locales, humedad, temperatura e
    // índice de calor en grados centígrados y Fahrenheit:
    // temperatura.html. Recibe como argumentos:
    // - LittleFS: Indica que los archivos a servir se
    // encuentran en el sistema de archivos LittleFS
    // - temperatura.html, Nombre del archivo a enviar
    // - "text/html": El tipo del contenido del archivo:
    // texto plano, codigo HTML.
    // - false: No se descarga el archivo
    // - processor: La función que insertará en la página web,
    // los valores de la fecha/hora, humedad, temperatura e
    // índice de calor en grados centígrados y Fahrenheit.
    request->send(LittleFS, "/temperatura.html", "text/html",
                  false, processor);
  });
  // Si se invoca al servidor con la URL:
  // "direccionIP_ServidorWev/c"
  // O si se presiona el el boton "Recargar"
  // en la pagina temperaturaCent.html
  server.on("/c", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Le envia al cliente el mensaje de respuesta con la pagina
    // con la fecha y hora locales, humedad, temperatura e
    // índice de calor en grados centígrados:
    // temperaturaCent.html.
    // Recibe como argumentos:
    // - LittleFS: Indica que los archivos a servir se
    // encuentran en el sistema de archivos LittleFS
    // - temperaturaCent.html, Nombre del archivo a enviar
    // - "text/html": El tipo del contenido del archivo:
    // texto plano, codigo HTML.
    // - false: No se descarga el archivo
    // - processor: La función que insertará en la página web,
    // los valores de la fecha/hora, humedad, temperatura e
    // índice de calor en grados centígrados.
    request->send(LittleFS, "/temperaturaCent.html",
                  "text/html", false, processor);
  });
  // Si se invoca al servidor con la URL:
  // "direccionIP_ServidorWev/f"
  // O si se presiona el el boton "Recargar"
  // en la pagina temperaturaFahr.html
  server.on("/f", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Le envia al cliente el mensaje de respuesta con la pagina
    // con la fecha y hora locales, humedad, temperatura e
    // índice de calor en grados Fahrenheit:
    // temperaturaFahr.html. Recibe como argumentos:
    // - LittleFS: Indica que los archivos a servir se
    // encuentran en el sistema de archivos LittleFS
    // - temperaturaFahr.html, Nombre del archivo a enviar
    // - "text/html": El tipo del contenido del archivo:
    // texto plano, codigo HTML.
    // - false: No se descarga el archivo
    // - processor: La función que insertará en la página web,
    // los valores de la fecha/hora, humedad, temperatura e
    // índice de calor en grados Fahrenheit.
    request->send(LittleFS, "/temperaturaFahr.html",
                  "text/html", false, processor);
  });
  // Invoca a la funcion noHallada() si se invoca al
  // servidor con una URL inexistente
  server.onNotFound(noHallada);
}
/*
 * Esta funcion le envia al cliente una pagina
 * con el mensaje de que la URL solicitada no se encontro
 */
void noHallada(AsyncWebServerRequest* request) {
  // Le envia al cliente el mensaje de respuesta. Recibe como
  // argumentos:
  // - 404: El codigo de estado HTTP (indica que no se pudo
  // atender la solicitud).
  // - "text/plain": El tipo del contenido del mensaje (texto
  // plano.
  // - "URL no encontrada": El cuerpo del mensaje de respuesta,
  // una cadena con el mensaje "URL no encontrada".
  request->send(404, "text/plain", "URL no encontrada");
}
/*
 * Esta funcion obtiene del reloj interno del microcontrolador
 * la fecha y hora y lee el sensor DHT11. Toma alrededor de
 * 250 ms
 */
void actualizaLectura() {
  // lee la fecha y hora del sistema
  fecha = obtenFecha();
  // Lee el valor de la humedad
  humedad = obtenHumedad();
  // Lee el valor de la temperatura en grados centígrados
  temperaturaCent = obtenTempCent();
  // Lee el valor de la temperatura en grados Fahrenheith
  temperaturaFahr = obtenTempFahr();
  // Lee el valor del indice de calor en grados centígrados
  indiceCalorCent = obtenIndiceCalorCent();
  // Lee el valor del indice de calor en grados Fahrenheith
  indiceCalorFahr = obtenIndiceCalorFahr();
}
/*
 * Esta funcion obtiene la fecha y hora locales del sistema
 * Regresa la fecha y hora como cadena si hubo exito,
 * una cadena vacia en caso contrario
 */
String obtenFecha() {
  char sFecha[80];
  // Obtiene la fecha y hora actual
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No se pudo obtener la fecha/hora");
    sFecha[0] = '\0';
  } else {
    strftime(sFecha, 80, "%d/%m/%Y %H:%M:%S", &timeinfo);
  }
  return String(sFecha);
}
/*
 * Esta funcion lee la humedad usando el sensor DHT11.
 * Toma alrededor de 250 ms
 */
String obtenHumedad() {
  // Lee la humedad
  float humedad = dht.readHumidity();
  // Verifica que la lectura sea correcta
  if (isnan(humedad)) {
    Serial.println("Error al leer el sensor DHT11");
    return String();
  }
  return String(humedad);
}
/*
 * Esta funcion lee la temperatura en grados centigrados usando 
 * el sensor DHT11. Toma alrededor de 250 ms
 */
String obtenTempCent() {
  // Lee la temperatura en grados centigrados
  float tempCent = dht.readTemperature();
  // Verifica que la lectura sea correcta
  if (isnan(tempCent)) {
    Serial.println("Error al leer el sensor DHT11");
    return String();
  }
  return String(tempCent);
}
/*
 * Esta funcion lee la temperatura en grados Fahrenheit usando
 * el sensor DHT11. Toma alrededor de 250 ms
 */
String obtenTempFahr() {
  // Lee la temperatura en grados Fahrenheit
  float tempFahr = dht.readTemperature(true);
  // Verifica que la lectura sea correcta
  if (isnan(tempFahr)) {
    Serial.println("Error al leer el sensor DHT11");
    return String();
  }
  return String(tempFahr);
}
/*
 * Esta funcion calcula el indice de calor en grados centigrados
 * usando el sensor DHT11. Toma alrededor de 250 ms
 */
String obtenIndiceCalorCent() {
  // Lee la humedad
  float humedad = dht.readHumidity();
  // Lee la temperatura en grados centigrados
  float tempCent = dht.readTemperature();
  // Verifica que las lecturas sean correctas
  if (isnan(humedad) || isnan(tempCent)) {
    Serial.println("Error al leer el sensor DHT11");
    return String();
  }
  // Calcula el indice de calor en grados centigrados
  float indiceCalorCent = dht.computeHeatIndex(tempCent,
                                               humedad, false);
  return String(indiceCalorCent);
}
/*
 * Esta funcion calcula el indice de calor en grados Fahrenheit
 * usando el sensor DHT11. Toma alrededor de 250 ms
 */
String obtenIndiceCalorFahr() {
  // Lee la humedad
  float humedad = dht.readHumidity();
  // Lee la temperatura en grados centigrados
  float tempFahr = dht.readTemperature(true);
  // Verifica que las lecturas sean correctas
  if (isnan(humedad) || isnan(tempFahr)) {
    Serial.println("Error al leer el sensor DHT11");
    return String();
  }
  // Calcula el indice de calor en grados centigrados
  float indiceCalorCent = dht.computeHeatIndex(tempFahr, humedad);
  return String(indiceCalorCent);
}
/*
 * Esta funcion sustituye cada elemento incrustado en una pagina
 * de la forma %ELEMENTO% por una cadena calculada cuando la
 * condicion var == "ELEMENTO" se cumple. El parametro var
 * contiene uno de los elementos incrustados
 */
String processor(const String& var) {
  if (var == "FECHA")
    return fecha;
  if (var == "HUMEDAD")
    return humedad;
  if (var == "TEMP_CENT")
    return temperaturaCent;
  if (var == "TEMP_FAHR")
    return temperaturaFahr;
  if (var == "IND_CAL_CENT")
    return indiceCalorCent;

  if (var == "IND_CAL_FAHR")
    return indiceCalorFahr;

  return String();
}
