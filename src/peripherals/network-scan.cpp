#include <WiFi.h>
#include "network-scan.h"

boolean NetworkScan::scanning = false;
boolean NetworkScan::results = false;
int NetworkScan::resultCount = 0;
ScanResult NetworkScan::list[NETWORK_SCAN_MAX_RESULTS];

void NetworkScan::start() {
  if (scanning) {
    return;
  }
  WiFi.scanDelete();
  WiFi.scanNetworks(true, true);
  scanning = true;
}

void NetworkScan::loop() {
  if (!scanning) {
    return;
  }
  int found = WiFi.scanComplete();
  if (found == WIFI_SCAN_RUNNING) {
    return;
  }
  scanning = false;
  if (found < 0) {
    return;
  }

  resultCount = 0;
  for (int i = 0; i < found; i++) {
    // Insertion into the top-N list, ordered by decreasing RSSI
    int rssi = WiFi.RSSI(i);
    int position = resultCount;
    while (position > 0 && list[position - 1].rssi < rssi) {
      position--;
    }
    if (position >= NETWORK_SCAN_MAX_RESULTS) {
      continue;
    }
    int last = min(resultCount, NETWORK_SCAN_MAX_RESULTS - 1);
    for (int j = last; j > position; j--) {
      list[j] = list[j - 1];
    }
    String ssid = WiFi.SSID(i);
    strlcpy(list[position].ssid, ssid.length() ? ssid.c_str() : "(masque)", sizeof(list[position].ssid));
    list[position].rssi = rssi;
    list[position].channel = WiFi.channel(i);
    resultCount = min(resultCount + 1, NETWORK_SCAN_MAX_RESULTS);
  }
  WiFi.scanDelete();
  results = true;
}

boolean NetworkScan::isScanning() {
  return scanning;
}

boolean NetworkScan::hasResults() {
  return results;
}

int NetworkScan::count() {
  return resultCount;
}

const ScanResult &NetworkScan::result(int index) {
  return list[index];
}
