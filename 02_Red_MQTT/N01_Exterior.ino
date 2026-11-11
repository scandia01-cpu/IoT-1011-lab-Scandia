/* EJERCICIO 2 - NODO COMPARTIDO (Wokwi)
   Toca solo la seccion CONFIGURACION; el resto es el modulo de comunicacion.
   Publica telemetria y escucha otros nodos por MQTT/WSS.
*/
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebSocketsClient.h>
#include <DHTesp.h>
#include <ArduinoJson.h>
#include <vector>
#include <math.h>

// === CONFIGURACION DEL ESTUDIANTE (3 conexiones) ===
const char* RENDER_HOST = "iot-backen-1011.onrender.com";
const char* NODO_ID = "N01_scandia";

// ================================
const char* EXPERIMENTO = "lab2026";
const char* ZONA = "exterior";
const int PIN_DHT = 15;
const int TOTAL = 0; // 30 ejercicio 1; sin limite ejercicio 2
const unsigned long INTERVALO = 5000;  // Cambia a 2000 o 7000 en la prueba 2.
const bool ESCUCHAR_COMPANEROS = true;  // false: solo publica.
const char* NODO_A_ESCUCHAR = "+";       // "+": todos; o "N02_ana": un solo nodo.


DHTesp dht;
WebSocketsClient ws;
String topicoPropio, filtroTodos, rondaReal;
unsigned long ultimoEnvio = 0, ultimaConsulta = 0, ultimoPing = 0;
int intentos = 0;
int mensajesRecibidos = 0;
bool mqttListo = false;

// Las siguientes funciones encapsulan MQTT. No hay que modificarlas.
void textoMQTT(std::vector<uint8_t>& p, const String& texto) {
  uint16_t n = texto.length();
  p.push_back(n >> 8); p.push_back(n & 255);
  for (uint16_t i=0; i<n; i++) p.push_back((uint8_t)texto[i]);
}

bool paqueteMQTT(uint8_t tipo, const std::vector<uint8_t>& cuerpo) {
  std::vector<uint8_t> b; b.push_back(tipo);
  size_t n = cuerpo.size();
  do {
    uint8_t d = n % 128; n /= 128;
    if (n) d |= 128;
    b.push_back(d);
  } while(n);
  b.insert(b.end(), cuerpo.begin(), cuerpo.end());
  return ws.isConnected() && ws.sendBIN(b.data(), b.size());
}

void conectarMQTT() {
  std::vector<uint8_t> p;
  textoMQTT(p, "MQTT");
  p.push_back(4); p.push_back(2); // version 3.1.1, sesion limpia
  p.push_back(0); p.push_back(40); // keepalive 40 s
  textoMQTT(p, String("wokwi-") + NODO_ID + "-" + String((uint32_t)esp_random(), HEX));
  paqueteMQTT(0x10, p);
}

void suscribirse() {
  std::vector<uint8_t> p;
  p.push_back(0); p.push_back(1); // packet ID
  textoMQTT(p, filtroTodos);
  p.push_back(0); // QoS 0
  paqueteMQTT(0x82, p);
}

void procesarMQTT(uint8_t* b, size_t n) {
  if (n < 2) return;
  uint8_t tipo = b[0] >> 4;
  size_t i = 1; unsigned int mul = 1;
  while (i < n) {
    uint8_t d = b[i++];
    if (!(d & 128)) break;
    mul *= 128;
    if (i > 5) return;
  }
  if (tipo == 2) { // CONNACK
    if (n >= i + 2 && b[i+1] == 0) {
      mqttListo = true;
      Serial.println("MQTT conectado sobre WSS/443");
      if (ESCUCHAR_COMPANEROS) suscribirse();
    } else Serial.println("MQTT CONNECT rechazado");
    return;
  }
  if (tipo == 9) { Serial.println("MQTT suscripcion activa"); return; }
  if (tipo == 13) return; // PINGRESP
  if (tipo != 3 || i + 2 > n) return;
  uint16_t k = ((uint16_t)b[i] << 8) | b[i+1]; i += 2;
  if (i + k > n) return;
  String topico = "";
  for (uint16_t j=0; j<k; j++) topico += (char)b[i+j];
  i += k;
  if ((b[0] & 0x06) != 0) { if (i + 2 > n) return; i += 2; }
  JsonDocument doc;
  if (deserializeJson(doc, b+i, n-i)) return;
  const char* origen = doc["nodo_id"] | "?";
  if (String(origen) == NODO_ID) return;
  float t = doc["temperatura_c"] | 0.0f;
  float h = doc["humedad_pct"] | 0.0f;
  mensajesRecibidos++;
  Serial.printf("RECIBIDO %d DE %s | T=%.1f C | H=%.1f %% | %s\n",
                mensajesRecibidos, origen, t, h, topico.c_str());
}

void eventoWS(WStype_t tipo, uint8_t* datos, size_t n) {
  if (tipo == WStype_CONNECTED) {
    Serial.println("WSS conectado: negociando MQTT...");
    conectarMQTT();
  } else if (tipo == WStype_DISCONNECTED) {
    mqttListo = false;
    Serial.println("WSS desconectado; reintentando...");
  } else if (tipo == WStype_BIN) {
    procesarMQTT(datos, n);
  }
}

void wifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.mode(WIFI_STA);
  WiFi.begin("Wokwi-GUEST", "", 6);
  Serial.print("WiFi: conectando");
  unsigned long inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis()-inicio < 20000) {
    Serial.print("."); delay(300);
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? " OK" : " SIN CONEXION");
}

String crearJSON(int sec) {
  TempAndHumidity v = dht.getTempAndHumidity();
  if (isnan(v.temperature) || isnan(v.humidity)) return "";
  float temp = v.temperature;
  float hum = v.humidity;
  temp += 2.5f * sinf(sec * 0.30f);
  hum = constrain(hum - 4.0f * sinf(sec * 0.30f), 0.0f, 100.0f);
  JsonDocument obj;
  obj["experimento"] = EXPERIMENTO;
  obj["nodo_id"] = NODO_ID;
  obj["zona"] = ZONA;
  obj["ronda"] = rondaReal;
  obj["secuencia"] = sec;
  obj["temperatura_c"] = temp;
  obj["humedad_pct"] = hum;
  String json;
  serializeJson(obj, json);
  return json;
}

void mostrarMetricas() {
  WiFiClientSecure seguro; seguro.setInsecure(); // Solo simulador
  HTTPClient h;
  String url = String("https://") + RENDER_HOST +
    "/metricas/serial?experimento=" + EXPERIMENTO +
    "&nodo_id=" + NODO_ID + "&ronda=" + rondaReal;
  if (!h.begin(seguro, url)) return;
  h.setTimeout(12000);
  int codigo = h.GET();
  Serial.printf("\nMETRICAS EN RENDER: HTTP %d\n", codigo);
  if (codigo == 200) Serial.println(h.getString());
  else Serial.printf("Error consulta metricas: %s\n", h.errorToString(codigo).c_str());
  h.end(); seguro.stop();
}

void publicar() {
  String json = crearJSON(intentos+1);
  if (json.isEmpty()) { Serial.println("DHT22 sin lectura"); return; }
  std::vector<uint8_t> p;
  textoMQTT(p, topicoPropio);
  for (size_t i=0; i<json.length(); i++) p.push_back((uint8_t)json[i]);
  bool ok = paqueteMQTT(0x30, p); // MQTT PUBLISH QoS 0 real
  intentos++;
  Serial.printf("MQTT %d%s | PUBLISH=%s | JSON=%u B\n",
                intentos, TOTAL ? "/30" : "", ok ? "SI" : "NO", json.length());
}

void setup() {
  Serial.begin(115200);
  delay(400);
  if (String(RENDER_HOST).startsWith("TU_") || String(NODO_ID).endsWith("_tunombre")) {
    Serial.println("CAMBIA NODO_ID");
    while (true) delay(1000);
  }
  dht.setup(PIN_DHT, DHTesp::DHT22);
  rondaReal = String("S") + String((uint32_t)esp_random(), HEX);
  topicoPropio = String("doctorado/") + EXPERIMENTO + "/" + NODO_ID + "/telemetria";
  filtroTodos = String("doctorado/") + EXPERIMENTO + "/" + NODO_A_ESCUCHAR + "/telemetria";
  wifi();
  ws.onEvent(eventoWS);
  ws.setReconnectInterval(5000);
  // El cuarto parametro es el fingerprint TLS (NULL para Wokwi); el quinto es el subprotocolo MQTT.
  ws.beginSSL(RENDER_HOST, 443, "/mqtt", NULL, "mqtt");
  Serial.println("PUBLICO EN: " + topicoPropio);
  Serial.println("ESCUCHO: " + filtroTodos);
  Serial.printf("CADA %lu ms | perfil %s\n", INTERVALO, ZONA);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) { wifi(); delay(500); return; }
  ws.loop(); // Mantiene viva la conexion y recibe publicaciones.
  if (mqttListo && (!TOTAL || intentos<TOTAL) && millis()-ultimoEnvio >= INTERVALO) {
    ultimoEnvio = millis(); publicar();
  }
  if (mqttListo && millis()-ultimoPing >= 18000) {
    ultimoPing = millis(); paqueteMQTT(0xC0, std::vector<uint8_t>());
  }
  if (TOTAL && intentos==TOTAL && millis()-ultimaConsulta >= 15000) {
    ultimaConsulta = millis(); mostrarMetricas();
  }
  delay(20);
}
