/* EJERCICIO 1 - HTTP | ESP32 + DHT22 (GPIO 15)
   Cambia solo NODO_ID. El backend calcula las metricas.
   Para repetir, usa otra RONDA (R02, R03...).
*/
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <DHTesp.h>
#include <ArduinoJson.h>

// === CAMBIAR SOLO ESTOS CAMPOS ===
const char* RENDER_HOST = "iot-backen-1011.onrender.com";
const char* NODO_ID = "N01_scandia";
const char* RONDA = "R01";  // Igual en HTTP y MQTT; cambiar al repetir
// ================================
const char* EXPERIMENTO = "lab2026";
const char* ZONA = "exterior";
const int PIN_DHT = 15;
const int TOTAL = 30;
const unsigned long INTERVALO = 2500;

DHTesp dht;
unsigned long ultimoEnvio = 0, ultimaConsulta = 0;
int intentos = 0;

void wifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.mode(WIFI_STA);
  WiFi.begin("Wokwi-GUEST", "", 6);
  Serial.print("WiFi: conectando");
  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 20000) {
    Serial.print("."); delay(300);
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " OK" : " SIN CONEXION");
}

String medicion(int numero) {
  TempAndHumidity v = dht.getTempAndHumidity();
  if (isnan(v.temperature) || isnan(v.humidity)) return "";
  JsonDocument obj;
  obj["experimento"] = EXPERIMENTO;
  obj["nodo_id"] = NODO_ID;
  obj["zona"] = ZONA;
  obj["ronda"] = RONDA;
  obj["secuencia"] = numero;
  obj["temperatura_c"] = v.temperature;
  obj["humedad_pct"] = v.humidity;
  String json;
  serializeJson(obj, json);
  return json;
}

void mostrarMetricas() {
  WiFiClientSecure seguro;
  seguro.setInsecure(); // Solo laboratorio simulado
  HTTPClient h;
  String url = String("https://") + RENDER_HOST + "/metricas/serial?experimento=" +
               EXPERIMENTO + "&nodo_id=" + NODO_ID + "&ronda=" + RONDA;
  if (!h.begin(seguro, url)) { Serial.println("No se pudo abrir el GET de metricas"); return; }
  h.setTimeout(12000);
  int codigo = h.GET();
  Serial.printf("\nMETRICAS HTTP GET: %d\n", codigo);
  if (codigo == 200) Serial.println(h.getString());
  else Serial.printf("Consulta fallo: %s\n", h.errorToString(codigo).c_str());
  h.end();
  seguro.stop();
}

void enviarHTTP() {
  String json = medicion(intentos + 1);
  if (json.isEmpty()) { Serial.println("DHT22 sin lectura"); return; }
  WiFiClientSecure seguro;
  seguro.setInsecure(); // Solo simulador Wokwi
  HTTPClient h;
  String url = String("https://") + RENDER_HOST + "/mediciones";
  int codigo = -999;
  if (h.begin(seguro, url)) {
    h.setTimeout(12000);
    h.addHeader("Content-Type", "application/json");
    codigo = h.POST(json);
  }
  intentos++; // Intentos: incluye fallidos. Render cuenta solo recibidos unicos.
  Serial.printf("HTTP %d/%d | codigo=%d | JSON=%u B\n", intentos, TOTAL, codigo, json.length());
  if (codigo < 0) Serial.printf("Detalle HTTP: %s\n", h.errorToString(codigo).c_str());
  h.end();
  seguro.stop();
}

void setup() {
  Serial.begin(115200);
  delay(400);
  if (String(RENDER_HOST).startsWith("TU_") || String(NODO_ID).endsWith("_tunombre")) {
    Serial.println("CAMBIA NODO_ID EN EL SKETCH");
    while (true) delay(1000);
  }
  dht.setup(PIN_DHT, DHTesp::DHT22);
  wifi();
  Serial.println("HTTP: 30 mediciones. La API calculara las metricas.");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) { wifi(); delay(500); return; }
  if (intentos < TOTAL && millis() - ultimoEnvio >= INTERVALO) {
    ultimoEnvio = millis(); enviarHTTP();
  }
  if (intentos == TOTAL && millis() - ultimaConsulta >= 15000) {
    ultimaConsulta = millis(); mostrarMetricas();
  }
  delay(30);
}
