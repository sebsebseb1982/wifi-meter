#include "light-sensor.h"
#include "debug.h"
#include "pinout.h"

float LightSensor::smoothed = 0;
int LightSensor::lastRaw = 0;
boolean LightSensor::valid = false;
unsigned long LightSensor::lastSampleMs = 0;

void LightSensor::setup() {
  pinMode(LIGHT_SENSOR_PIN, INPUT);
}

void LightSensor::loop() {
  if (millis() - lastSampleMs < LIGHT_SAMPLE_PERIOD_MS) {
    return;
  }
  lastSampleMs = millis();

  lastRaw = analogRead(LIGHT_SENSOR_PIN);
  float percent = lastRaw * 100.0f / 4095.0f;
#if LIGHT_SENSOR_INVERTED
  percent = 100.0f - percent;
#endif
  if (valid) {
    smoothed += LIGHT_SMOOTHING_ALPHA * (percent - smoothed);
  } else {
    smoothed = percent;
    valid = true;
  }
  DEBUG_PRINT("light raw=");
  DEBUG_PRINTLN(lastRaw);
}

boolean LightSensor::available() {
  return valid;
}

int LightSensor::level() {
  return constrain((int)lroundf(smoothed), 0, 100);
}

int LightSensor::raw() {
  return lastRaw;
}
