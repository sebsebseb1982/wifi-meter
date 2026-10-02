#include "flash-led.h"
#include "pinout.h"
#include "signal-strength.h"
#include "wifi-connection.h"

#define FLASH_PWM_CHANNEL 7
#define FLASH_PWM_FREQUENCY 5000
#define FLASH_PWM_RESOLUTION 8

unsigned long FlashLed::lastPulseMs = 0;
boolean FlashLed::lit = false;

void FlashLed::setup() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(FLASH_LED_PIN, FLASH_PWM_FREQUENCY, FLASH_PWM_RESOLUTION);
#else
  ledcSetup(FLASH_PWM_CHANNEL, FLASH_PWM_FREQUENCY, FLASH_PWM_RESOLUTION);
  ledcAttachPin(FLASH_LED_PIN, FLASH_PWM_CHANNEL);
#endif
  write(0);
}

void FlashLed::write(uint8_t duty) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(FLASH_LED_PIN, duty);
#else
  ledcWrite(FLASH_PWM_CHANNEL, duty);
#endif
}

void FlashLed::loop() {
  if (!WiFiConnection::isConnected() || !SignalStrength::hasValue()) {
    if (lit) {
      write(0);
      lit = false;
    }
    return;
  }

  unsigned long now = millis();
  if (lit) {
    if (now - lastPulseMs >= FLASH_PULSE_MS) {
      write(0);
      lit = false;
    }
    return;
  }

  // The stronger the signal, the shorter the period
  long period = map(SignalStrength::quality(), 0, 100, FLASH_PERIOD_WEAK_MS, FLASH_PERIOD_STRONG_MS);
  if (now - lastPulseMs >= (unsigned long)period) {
    write(FLASH_BRIGHTNESS);
    lit = true;
    lastPulseMs = now;
  }
}
