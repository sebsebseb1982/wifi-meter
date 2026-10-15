#ifndef NETWORK_SCAN_H
#define NETWORK_SCAN_H

#include <Arduino.h>

#define NETWORK_SCAN_MAX_RESULTS 6

struct ScanResult {
  char ssid[33];
  int rssi;
  int channel;
};

class NetworkScan {
  public:
    static void loop();
    // Starts an asynchronous scan (ignored if one is already running)
    static void start();
    static boolean isScanning();
    static boolean hasResults();
    static int count();
    // Results are sorted by decreasing RSSI
    static const ScanResult &result(int index);

  private:
    static boolean scanning;
    static boolean results;
    static int resultCount;
    static ScanResult list[NETWORK_SCAN_MAX_RESULTS];
};

#endif
