#include "JsonResponses.h"

#include <ArduinoJson.h>
#include <WiFi.h>

String getIdentityJson(const String &deviceId) {
  JsonDocument doc;

  doc["id"] = deviceId;
  doc["hostname"] = deviceId + ".local";
  doc["model"] = "relayrun-4";
  doc["channels"] = 4;
  doc["firmware"] = "0.1.0";
  doc["mac"] = WiFi.macAddress();

  String output;
  serializeJson(doc, output);

  return output;
}

String getStatusJson(const char *status) {
  JsonDocument doc;

  doc["status"] = status;

  String output;
  serializeJson(doc, output);

  return output;
}

String getRelayStatusJson(bool relay1On, bool relay2On, uint32_t uptimeMs,
                          const String &ipAddress) {
  JsonDocument doc;

  doc["status"] = "running";
  doc["relay_1"] = relay1On ? "on" : "off";
  doc["relay_2"] = relay2On ? "on" : "off";
  doc["uptime_ms"] = uptimeMs;
  doc["ip"] = ipAddress;

  String output;
  serializeJson(doc, output);

  return output;
}
