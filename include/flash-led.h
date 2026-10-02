#ifndef FLASH_LED_H
#define FLASH_LED_H

#include <Arduino.h>

// The flash LED is very bright: keep the PWM duty low (0..255)
#define FLASH_BRIGHTNESS 20
// Duration of each flash
#define FLASH_PULSE_MS 25
// Blink period for the weakest signal (0%) and the strongest one (100%)
#define FLASH_PERIOD_WEAK_MS 1500
#define FLASH_PERIOD_STRONG_MS 80

class FlashLed {
  public:
    static void setup();
    static void loop();

  private:
    static unsigned long lastPulseMs;
    static boolean lit;
    static void write(uint8_t duty);
};

#endif
