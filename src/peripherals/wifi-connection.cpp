#include <WiFi.h>
#include "wifi-connection.h"
#include "debug.h"
#include "secrets.h"

// Restart the association if it has not succeeded within this delay
#define WIFI_CONNECT_TIMEOUT_MS 15000

unsigned int WiFiConnection::nbConnection = 0;
WiFiState WiFiConnection::state = WIFI_STATE_CONNECTING;
unsigned long WiFiConnection::connectStartMs = 0;

void WiFiConnection::setup() {
  DEBUG_PRINTLN("WiFiConnection::setup()");
  WiFi.mode(WIFI_STA);
  // No modem sleep: keeps RSSI measurements frequent and accurate
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  begin();
}

void WiFiConnection::begin() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  state = WIFI_STATE_CONNECTING;
  connectStartMs = millis();
}

void WiFiConnection::loop() {
  switch (state) {
    case WIFI_STATE_CONNECTED:
      if (WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINTLN("WiFi disconnected, reconnecting...");
        state = WIFI_STATE_CONNECTING;
        connectStartMs = millis();
      }
      break;

    case WIFI_STATE_CONNECTING:
      if (WiFi.status() == WL_CONNECTED) {
        nbConnection++;
        state = WIFI_STATE_CONNECTED;
        DEBUG_PRINT("WiFi connected: ");
        DEBUG_PRINTLN(WiFi.localIP());
      } else if (millis() - connectStartMs >= WIFI_CONNECT_TIMEOUT_MS) {
        DEBUG_PRINTLN("WiFi connection timeout, retrying...");
        WiFi.disconnect();
        begin();
      }
      break;
  }
}

boolean WiFiConnection::isConnected() {
  return state == WIFI_STATE_CONNECTED;
}

unsigned long WiFiConnection::connectingSinceMs() {
  return millis() - connectStartMs;
}

const char *WiFiConnection::statusText() {
  switch (WiFi.status()) {
    case WL_CONNECTED:       return "Connecte";
    case WL_NO_SSID_AVAIL:   return "SSID introuvable";
    case WL_CONNECT_FAILED:  return "Echec connexion";
    case WL_CONNECTION_LOST: return "Connexion perdue";
    case WL_DISCONNECTED:    return "Deconnecte";
    default:                 return "Recherche...";
  }
}
