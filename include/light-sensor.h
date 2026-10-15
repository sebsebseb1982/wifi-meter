#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <Arduino.h>

#define LIGHT_SAMPLE_PERIOD_MS 200
#define LIGHT_SMOOTHING_ALPHA 0.2f
// Set to 1 if the reading goes down when the ambient light goes up
#define LIGHT_SENSOR_INVERTED 0

class LightSensor {
  public:
    static void setup();
    static void loop();
    // False when the ADC cannot be read (ADC2 is unavailable while the WiFi is on)
    static boolean available();
    // Ambient light in percent (0 = dark, 100 = bright)
    static int level();
    static int raw();

  private:
    static float smoothed;
    static int lastRaw;
    static boolean valid;
    static unsigned long lastSampleMs;
};

#endif
