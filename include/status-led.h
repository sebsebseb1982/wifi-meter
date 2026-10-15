#ifndef STATUS_LED_H
#define STATUS_LED_H

#include <Arduino.h>

// Duration of each blink
#define STATUS_LED_PULSE_MS 25
// Blink period for the weakest signal (0%) and the strongest one (100%)
#define STATUS_LED_PERIOD_WEAK_MS 1500
#define STATUS_LED_PERIOD_STRONG_MS 80

// Blink half period while the WiFi configuration portal is open
#define STATUS_LED_PORTAL_HALF_PERIOD_MS 400

class StatusLed {
  public:
    static void setup();
    static void loop();
    static void toggleMute();
    static boolean isMuted();

  private:
    static unsigned long lastPulseMs;
    static boolean lit;
    static boolean muted;
    static void write(boolean on);
};

#endif
