#ifndef SIGNAL_STRENGTH_H
#define SIGNAL_STRENGTH_H

#include <Arduino.h>

// RSSI sampling period
#define SIGNAL_SAMPLE_PERIOD_MS 250
// Exponential smoothing factor (0 < alpha <= 1, 1 = no smoothing)
#define SIGNAL_SMOOTHING_ALPHA 0.4f

class SignalStrength {
  public:
    static const int HISTORY_SIZE = 128;

    static void loop();
    static boolean hasValue();
    // Smoothed RSSI in dBm
    static int rssi();
    // Signal quality in percent (0..100), derived from RSSI
    static int quality();
    // Number of bars (0..5)
    static int bars();
    // Quality history, index 0 is the oldest sample
    static int historyCount();
    static int history(int index);

  private:
    static float smoothedRssi;
    static boolean valid;
    static unsigned long lastSampleMs;
    static uint8_t historyBuffer[HISTORY_SIZE];
    static int historyStart;
    static int historyLength;
    static void push(int quality);
};

#endif
