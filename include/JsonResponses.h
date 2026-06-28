#pragma once

#include <Arduino.h>

String getIdentityJson(const String &deviceId);
String getStatusJson(const char *status);
String getRelayStatusJson(bool relay1On, bool relay2On, uint32_t uptimeMs,
                          const String &ipAddress);
