#include <WiFi.h>
#include <Preferences.h>
#include "wifi-connection.h"
#include "wifi-portal.h"
#include "debug.h"

// Restart the association if it has not succeeded within this delay
#define WIFI_CONNECT_TIMEOUT_MS 15000

unsigned int WiFiConnection::nbConnection = 0;
WiFiState WiFiConnection::state = WIFI_STATE_CONNECTING;
unsigned long WiFiConnection::connectStartMs = 0;
unsigned long WiFiConnection::portalStartMs = 0;
int WiFiConnection::attempts = 1;
char WiFiConnection::savedSsid[33] = "";
char WiFiConnection::savedPassword[65] = "";

void WiFiConnection::setup() {
  DEBUG_PRINTLN("WiFiConnection::setup()");
  WiFi.mode(WIFI_STA);
  // No modem sleep: keeps RSSI measurements frequent and accurate
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);

  loadCredentials();
  if (savedSsid[0] == '\0') {
    startPortal();
  } else {
    begin();
  }
}

void WiFiConnection::loadCredentials() {
  Preferences preferences;
  preferences.begin("wifi", false);
  preferences.getString("ssid", savedSsid, sizeof(savedSsid));
  preferences.getString("password", savedPassword, sizeof(savedPassword));
  preferences.end();
}

void WiFiConnection::saveCredentials(const String &ssid, const String &password) {
  Preferences preferences;
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();
  strlcpy(savedSsid, ssid.c_str(), sizeof(savedSsid));
  strlcpy(savedPassword, password.c_str(), sizeof(savedPassword));
}

void WiFiConnection::begin() {
  WiFi.begin(savedSsid, savedPassword);
  state = WIFI_STATE_CONNECTING;
  connectStartMs = millis();
}

void WiFiConnection::startPortal() {
  DEBUG_PRINTLN("Starting the captive portal");
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  WiFiPortal::start();
  state = WIFI_STATE_PORTAL;
  portalStartMs = millis();
}

void WiFiConnection::leavePortal() {
  WiFiPortal::stop();
  WiFi.mode(WIFI_STA);
  attempts = 1;
  begin();
}

void WiFiConnection::loop() {
  switch (state) {
    case WIFI_STATE_CONNECTED:
      if (WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINTLN("WiFi disconnected, reconnecting...");
        state = WIFI_STATE_CONNECTING;
        attempts = 1;
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
        if (attempts >= WIFI_MAX_ATTEMPTS) {
          DEBUG_PRINTLN("WiFi connection failed, opening the portal");
          startPortal();
        } else {
          DEBUG_PRINTLN("WiFi connection timeout, retrying...");
          attempts++;
          WiFi.disconnect();
          begin();
        }
      }
      break;

    case WIFI_STATE_PORTAL:
      WiFiPortal::loop();
      // Nobody configured the network: try the saved one again
      if (savedSsid[0] != '\0' && WiFiPortal::clientCount() == 0 &&
          millis() - portalStartMs >= WIFI_PORTAL_TIMEOUT_MS) {
        leavePortal();
      }
      break;
  }
}

boolean WiFiConnection::isConnected() {
  return state == WIFI_STATE_CONNECTED;
}

boolean WiFiConnection::isPortal() {
  return state == WIFI_STATE_PORTAL;
}

unsigned long WiFiConnection::connectingSinceMs() {
  return millis() - connectStartMs;
}

const char *WiFiConnection::ssid() {
  return savedSsid;
}

int WiFiConnection::attempt() {
  return attempts;
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
