#include <NoDelay.h>
#include "DHT.h"
// Pin a la que esta conectado el sensor DHT11, GPIO4
const unsigned int PIN_DHT = 4;
// Velocidad de transmisión del puerto serie
const unsigned int BAUD_RATE = 115200;
// Tiempo entre lecturas del sensor DHT11
const long PAUSA = 2000;
// Crea una instancia de la clase noDelay
// que determina si han transcurrido PERIODO ms
noDelay pausa(PAUSA);
// Crea una instancia de la clase DHT11
DHT dht(PIN_DHT, DHT11);
// Estructura para almacenar las lecturas del sensor DHT11
typedef struct {
  float humedad;
  float temperaturaCent;
  float temperaturaFahr;
  float indiceCalorCent;
  float indiceCalorFahr;
} sDht11;
// Variable que almacena las lecturas del sensor DHT11
sDht11 sdht11;
void actualizaLectura();
bool readDht11(sDht11 *psDht11);
void printValues(sDht11 *psDht11);
void setup() {
  // Establece la velocidad de transmisión del puerto
  // serie al valor BAUD_RATE
  Serial.begin(BAUD_RATE);
  delay(100);
  Serial.println("Prueba del sensor DHT11");
  // Inicializa el sensor DHT11
  dht.begin();
  Serial.println();
}
void loop() {
  // Verifica si es tiempo de leer el sensor DHT11
  if (pausa.update()) {
    // Lee el sensor y despliega la humedad y temperatura
    // en el monitor serial
    actualizaLectura();
  }
}
void actualizaLectura() {
  // Lee el sensor DHT11
  if (readDht11(&sdht11))
    // Despliega la humedad y temperatura
    printValues(&sdht11);
}
/*
 * Esta funcion lee el sensor DHT11 y guarda los datos
 * en la estructura sDht11. Toma alrededor de 250 ms
 */
bool readDht11(sDht11 *psDht11) {
  // Lee la humedad
  float humedad = dht.readHumidity();
  // Lee la temperatura en grados centigrados
  float tempCent = dht.readTemperature();
  // Lee la temperatura en grados Fahrenheit
  float tempFahr = dht.readTemperature(true);
  // Verifica que las lecturas sean correctas
  if (isnan(humedad) || isnan(tempCent) || isnan(tempFahr)) {
    Serial.println("Error al leer el sensor DHT11");
    return false;
  }
  // Calcula el indice de calor en grados centigrados
  float indiceCalorCent = dht.computeHeatIndex(tempCent, humedad,
                                               false);
  // Calcula el indice de calor en grados Fahrenheit
  float indiceCalorFahr = dht.computeHeatIndex(tempFahr,
                                               humedad);
  // Guarda las lecturas en la estructura apuntada por psDht11
  psDht11->humedad = humedad;
  psDht11->temperaturaCent = tempCent;
  psDht11->temperaturaFahr = tempFahr;
  psDht11->indiceCalorCent = indiceCalorCent;
  psDht11->indiceCalorFahr = indiceCalorFahr;
  return true;
}
/*
 * Esta funcion despliega en el monitor serial del IDE de
 * Arduino las lecturas obtenidas del sensor DHT11
 */
void printValues(sDht11 *psDht11) {
  Serial.print("Humedad = ");
  Serial.print(psDht11->humedad);
  Serial.println(" %");
  Serial.print("Temperatura = ");
  Serial.print(psDht11->temperaturaCent);
  Serial.println(" *C");

  Serial.print("Temperatura = ");
  Serial.print(psDht11->temperaturaFahr);
  Serial.println(" *F");
  Serial.print("Indice de Calor = ");
  Serial.print(psDht11->indiceCalorCent);
  Serial.println(" *C");
  Serial.print("Indice de Calor = ");
  Serial.print(psDht11->indiceCalorFahr);
  Serial.println(" *F");
  Serial.println();
}