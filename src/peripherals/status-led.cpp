#include "status-led.h"
#include "pinout.h"
#include "signal-strength.h"
#include "wifi-connection.h"

unsigned long StatusLed::lastPulseMs = 0;
boolean StatusLed::lit = false;
boolean StatusLed::muted = false;

void StatusLed::setup() {
  pinMode(STATUS_LED_PIN, OUTPUT);
  write(false);
}

void StatusLed::write(boolean on) {
  digitalWrite(STATUS_LED_PIN, on ? HIGH : LOW);
  lit = on;
}

void StatusLed::toggleMute() {
  muted = !muted;
}

boolean StatusLed::isMuted() {
  return muted;
}

void StatusLed::loop() {
  // Captive portal open: steady slow blink, whatever the mute setting
  if (WiFiConnection::isPortal()) {
    if (millis() - lastPulseMs >= STATUS_LED_PORTAL_HALF_PERIOD_MS) {
      write(!lit);
      lastPulseMs = millis();
    }
    return;
  }

  if (muted || !WiFiConnection::isConnected() || !SignalStrength::hasValue()) {
    if (lit) {
      write(false);
    }
    return;
  }

  unsigned long now = millis();
  if (lit) {
    if (now - lastPulseMs >= STATUS_LED_PULSE_MS) {
      write(false);
    }
    return;
  }

  // The stronger the signal, the shorter the period
  long period = map(SignalStrength::quality(), 0, 100, STATUS_LED_PERIOD_WEAK_MS, STATUS_LED_PERIOD_STRONG_MS);
  if (now - lastPulseMs >= (unsigned long)period) {
    write(true);
    lastPulseMs = now;
  }
}
