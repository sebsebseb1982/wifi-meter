#include <WiFi.h>
#include "signal-strength.h"
#include "wifi-connection.h"

float SignalStrength::smoothedRssi = -100;
boolean SignalStrength::valid = false;
unsigned long SignalStrength::lastSampleMs = 0;
uint8_t SignalStrength::historyBuffer[HISTORY_SIZE];
int SignalStrength::historyStart = 0;
int SignalStrength::historyLength = 0;
int SignalStrength::statMin = 0;
int SignalStrength::statMax = -127;
long SignalStrength::statSum = 0;
long SignalStrength::statCount = 0;

void SignalStrength::loop() {
  if (millis() - lastSampleMs < SIGNAL_SAMPLE_PERIOD_MS) {
    return;
  }
  lastSampleMs = millis();

  if (!WiFiConnection::isConnected()) {
    valid = false;
    push(0);
    return;
  }

  int raw = WiFi.RSSI();
  if (raw == 0) {
    // 0 means "no measurement available yet"
    return;
  }

  if (valid) {
    smoothedRssi += SIGNAL_SMOOTHING_ALPHA * (raw - smoothedRssi);
  } else {
    smoothedRssi = raw;
    valid = true;
  }
  push(quality());

  int current = rssi();
  statMin = min(statMin, current);
  statMax = max(statMax, current);
  statSum += current;
  statCount++;
}

int SignalStrength::minRssi() {
  return statCount ? statMin : 0;
}

int SignalStrength::maxRssi() {
  return statCount ? statMax : 0;
}

int SignalStrength::averageRssi() {
  return statCount ? (int)lroundf((float)statSum / statCount) : 0;
}

void SignalStrength::resetStats() {
  statMin = 0;
  statMax = -127;
  statSum = 0;
  statCount = 0;
}

boolean SignalStrength::hasValue() {
  return valid;
}

int SignalStrength::rssi() {
  return (int)lroundf(smoothedRssi);
}

int SignalStrength::quality() {
  if (!valid) {
    return 0;
  }
  // -100 dBm => 0%, -50 dBm => 100%
  return constrain(2 * (rssi() + 100), 0, 100);
}

int SignalStrength::bars() {
  int q = quality();
  if (q == 0) return 0;
  return constrain((q + 19) / 20, 1, 5);
}

int SignalStrength::historyCount() {
  return historyLength;
}

int SignalStrength::history(int index) {
  return historyBuffer[(historyStart + index) % HISTORY_SIZE];
}

void SignalStrength::push(int quality) {
  if (historyLength < HISTORY_SIZE) {
    historyBuffer[(historyStart + historyLength) % HISTORY_SIZE] = quality;
    historyLength++;
  } else {
    historyBuffer[historyStart] = quality;
    historyStart = (historyStart + 1) % HISTORY_SIZE;
  }
}
