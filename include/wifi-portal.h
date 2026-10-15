#ifndef WIFI_PORTAL_H
#define WIFI_PORTAL_H

#include <Arduino.h>

// Access point opened by the captive portal. Leave the password empty for an open network
// (the WiFi password typed in the portal then travels unencrypted)
#define WIFI_PORTAL_AP_NAME "WiFi-Meter"
#define WIFI_PORTAL_AP_PASSWORD ""
#define WIFI_PORTAL_MAX_NETWORKS 16

class WiFiPortal {
  public:
    static void start();
    static void stop();
    static void loop();
    // Number of devices connected to the access point
    static int clientCount();
    static const char *apName();
    static String apAddress();
};

#endif
