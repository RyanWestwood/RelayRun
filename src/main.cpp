#include <Arduino.h>
#if defined(ESP32) || defined(LIBRETINY)
#include <AsyncTCP.h>
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#elif defined(TARGET_RP2040) || defined(TARGET_RP2350) ||                      \
    defined(PICO_RP2040) || defined(PICO_RP2350)
#include <RPAsyncTCP.h>
#include <WiFi.h>
#endif

#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <LittleFS.h>
#include <Update.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");           // access at ws://[esp ip]/ws
AsyncEventSource events("/events"); // event source (Server-Sent events)

const char *ssid = "westwood";
const char *password = "MadiRyan21";

// flag to use from web update to reboot the ESP
bool shouldReboot = false;

void onRequest(AsyncWebServerRequest *request) {
  // Handle Unknown Request
  request->send(404);
}

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len,
            size_t index, size_t total) {
  // Handle body
}

void onUpload(AsyncWebServerRequest *request, String filename, size_t index,
              uint8_t *data, size_t len, bool final) {
  // Handle upload
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
  // Handle WebSocket event
}

void setup() {
  Serial.begin(921600);

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.printf("WiFi Failed!\n");
    return;
  }
  Serial.printf("\nConnected! IP address: %s\n",
                WiFi.localIP().toString().c_str());

  // attach AsyncWebSocket
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // attach AsyncEventSource
  server.addHandler(&events);

  // respond to GET requests on URL /heap
  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  // upload a file to /upload
  server.on(
      "/upload", HTTP_POST,
      [](AsyncWebServerRequest *request) { request->send(200); }, onUpload);

  // Serve the local configuration page from LittleFS.
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response =
        request->beginResponse(LittleFS, "/index.htm.gz", "text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  // attach filesystem root at URL /fs (LittleFS example)
  server.serveStatic("/fs", LittleFS, "/");

  // Catch-All Handlers
  // Any request that can not find a Handler that canHandle it
  // ends in the callbacks below.
  server.onNotFound(onRequest);
  server.onFileUpload(onUpload);
  server.onRequestBody(onBody);

  server.begin();
}

void loop() {
  if (shouldReboot) {
    Serial.println("Rebooting...");
    delay(100);
    ESP.restart();
  }
  static char temp[128];
  sprintf(temp, "Seconds since boot: %u", millis() / 1000);
  events.send(temp, "time"); // send event "time"
}
