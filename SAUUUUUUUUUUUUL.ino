// ============================================================
// Servidor CoAP en ESP32 - Práctica de laboratorio
// Recursos: /sensor (GET) y /led (POST)
// Transporte: UDP, puerto 5683
// Librería: CoAPSimple (hirotakaster)
// Versión con Token en la respuesta (compatible con aiocoap)
// ============================================================

#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>

// --- Credenciales (red de 2.4 GHz) ---
// CAMBIA estos valores. No subas tu contraseña real a GitHub.
const char* ssid     = "Docentes-ITSOEH";
const char* password = "b0;(ka6,$1u-";

// --- Objetos globales ---
WiFiUDP udp;
Coap coap(udp);

// Pin del LED para el Reto 1 (opcional)
const int PIN_LED = 18;

// ============================================================
// CALLBACK /led
// ============================================================
void callback_led(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("Petición CoAP recibida en /led");

  // Copiar el payload (bytes sin '\0') a un texto
  char p[packet.payloadlen + 1];
  memcpy(p, packet.payload, packet.payloadlen);
  p[packet.payloadlen] = '\0';

  String payloadStr(p);
  Serial.print("Payload recibido: ");
  Serial.println(payloadStr);

  // Reto 1 (opcional): encender / apagar un LED real
  if (payloadStr == "ON")  digitalWrite(PIN_LED, HIGH);
  if (payloadStr == "OFF") digitalWrite(PIN_LED, LOW);

  // Respuesta: se reenvían Message ID Y Token de la petición
  // para que el cliente (aiocoap) la empareje correctamente.
  const char* resp = "Estado: OK desde ESP32 CoAP Server";
  coap.sendResponse(ip, port, packet.messageid,
                    resp, strlen(resp),
                    COAP_CONTENT, COAP_TEXT_PLAIN,
                    packet.token, packet.tokenlen);
}

// ============================================================
// CALLBACK /sensor
// ============================================================
void callback_sensor(CoapPacket &packet, IPAddress ip, int port) {
  Serial.println("GET /sensor recibido");

  // Temperatura simulada entre 25.4 y 30.3 °C
  float temperaturaSimulada = 25.4 + (random(0, 50) / 10.0);

  String respuesta = "Temp: " + String(temperaturaSimulada, 1) + " C";
  Serial.println(respuesta);

  coap.sendResponse(ip, port, packet.messageid,
                    respuesta.c_str(), respuesta.length(),
                    COAP_CONTENT, COAP_TEXT_PLAIN,
                    packet.token, packet.tokenlen);
}

// ============================================================
// setup()
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_LED, OUTPUT);   // Reto 1
  digitalWrite(PIN_LED, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Desactivar ahorro de energía del Wi-Fi: evita que la placa
  // pierda datagramas UDP entrantes mientras "duerme" la radio.
  WiFi.setSleep(false);

  Serial.println("\nConectado a la red");
  Serial.print("Dirección IP del Servidor CoAP: ");
  Serial.println(WiFi.localIP());

  // Registrar recursos
  coap.server(callback_led, "led");
  coap.server(callback_sensor, "sensor");

  // Arrancar servidor (UDP 5683)
  coap.start();
  Serial.println("Servidor CoAP iniciado en el puerto 5683");
}

// ============================================================
// loop(): solo escuchar. Sin delay() largos.
// ============================================================
void loop() {
  coap.loop();
}