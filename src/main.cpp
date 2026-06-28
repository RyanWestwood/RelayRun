#include <Arduino.h>
#include <ESPmDNS.h>
#include <JsonResponses.h>
#include <WebServer.h>
#include <WiFi.h>

constexpr const char *WIFI_SSID = "";
constexpr const char *WIFI_PASSWORD = "";
constexpr const char *MDNS_NAME = "relayrun";

String DEVICE_ID;
String DEVICE_HOSTNAME;

WebServer WEB_SERVER(80);

constexpr uint8_t RELAY_PIN_1 = 26;
constexpr uint8_t RELAY_PIN_2 = 25;

void initalizeBoard();
void initializeWiFi();
void initializeMDNS();
void initializeWebServer();
void setRelays(bool enabled);

String getDeviceId() {
  uint64_t chipId = ESP.getEfuseMac();

  char id[32];
  snprintf(id, sizeof(id), "relayrun-%04X%08X",
           static_cast<uint16_t>(chipId >> 32), static_cast<uint32_t>(chipId));

  return String(id);
}

void sendJson(int statusCode, const String &json) {
  WEB_SERVER.send(statusCode, "application/json", json);
}

void setup() {
  initalizeBoard();
  initializeWiFi();
  initializeMDNS();
  initializeWebServer();
}

void loop() { WEB_SERVER.handleClient(); }

void initializeRoutes() {
  WEB_SERVER.on("/", HTTP_GET, []() {
    WEB_SERVER.send(200, "text/plain", "RelayRun server is alive");
  });

  WEB_SERVER.on("/on", HTTP_GET, []() {
    setRelays(true);

    sendJson(200, getStatusJson("on"));
  });

  WEB_SERVER.on("/off", HTTP_GET, []() {
    setRelays(false);

    sendJson(200, getStatusJson("off"));
  });

  WEB_SERVER.on("/status", HTTP_GET, []() {
    sendJson(200, getRelayStatusJson(digitalRead(RELAY_PIN_1) == LOW,
                                     digitalRead(RELAY_PIN_2) == LOW, millis(),
                                     WiFi.localIP().toString()));
  });

  WEB_SERVER.on("/identity", HTTP_GET,
                []() { sendJson(200, getIdentityJson(DEVICE_ID)); });
}

void initalizeBoard() {
  DEVICE_ID = getDeviceId();
  DEVICE_HOSTNAME = DEVICE_ID + ".local";

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);

  digitalWrite(RELAY_PIN_1, HIGH);
  digitalWrite(RELAY_PIN_2, HIGH);

  Serial.begin(921600);
  delay(1000);
}

void initializeWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.printf("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.printf(".");
  }
  Serial.printf("\nConnected to WiFi");
  Serial.printf("\n- IP address: %s\n", WiFi.localIP().toString().c_str());
}

void initializeMDNS() {
  if (!MDNS.begin(DEVICE_ID.c_str())) {
    Serial.println("Error setting up MDNS!");
    return;
  }

  MDNS.setInstanceName(DEVICE_ID.c_str());

  MDNS.addService("http", "tcp", 80);

  MDNS.addServiceTxt("relayrun", "tcp", "id", DEVICE_ID.c_str());
  MDNS.addServiceTxt("relayrun", "tcp", "api", "line");
  MDNS.addServiceTxt("relayrun", "tcp", "commands",
                     "on,off,status,identity,help");
  MDNS.addServiceTxt("http", "tcp", "type", "relay-controller");
  MDNS.addServiceTxt("http", "tcp", "channels", "4");
  MDNS.addServiceTxt("http", "tcp", "api", "rest");
  MDNS.addServiceTxt("http", "tcp", "version", "0.0.1");

  Serial.println("MDNS responder configured and running");
  Serial.printf("- Access at: http://%s\n", DEVICE_HOSTNAME.c_str());
}

void initializeWebServer() {
  initializeRoutes();

  WEB_SERVER.begin();
  Serial.println("Web server started");
}

void setRelays(bool enabled) {
  digitalWrite(LED_BUILTIN, enabled ? HIGH : LOW);
  digitalWrite(RELAY_PIN_1, enabled ? LOW : HIGH);
  digitalWrite(RELAY_PIN_2, enabled ? LOW : HIGH);
}

String getRelayStatus() {
  return getRelayStatusJson(digitalRead(RELAY_PIN_1) == LOW,
                            digitalRead(RELAY_PIN_2) == LOW, millis(),
                            WiFi.localIP().toString());
}
