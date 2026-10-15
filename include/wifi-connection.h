#ifndef WIFI_CONNECTION_H
#define WIFI_CONNECTION_H

#include <Arduino.h>

// Failed association attempts before the configuration portal opens
#define WIFI_MAX_ATTEMPTS 3
// With saved credentials, the portal closes after this delay and the connection is retried
#define WIFI_PORTAL_TIMEOUT_MS 300000

enum WiFiState
{
    WIFI_STATE_CONNECTED,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_PORTAL
};

class WiFiConnection {
  public:
    static unsigned int nbConnection;
    static WiFiState state;
    static void setup();
    static void loop();
    static boolean isConnected();
    static boolean isPortal();
    static unsigned long connectingSinceMs();
    static const char *statusText();
    // Network being joined (saved in flash through the captive portal)
    static const char *ssid();
    static int attempt();
    // Opens the captive portal, whatever the current state
    static void startPortal();
    // Saves the credentials in flash, used by the portal
    static void saveCredentials(const String &ssid, const String &password);

  private:
    static unsigned long connectStartMs;
    static unsigned long portalStartMs;
    static int attempts;
    static char savedSsid[33];
    static char savedPassword[65];
    static void begin();
    static void loadCredentials();
    static void leavePortal();
};

#endif
