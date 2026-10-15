#include <Arduino.h>

#include "buttons.h"
#include "debug.h"
#include "light-sensor.h"
#include "network-scan.h"
#include "screen.h"
#include "signal-strength.h"
#include "status-led.h"
#include "wifi-connection.h"

void setup()
{
  Serial.begin(115200);
  DEBUG_PRINTLN("setup()");

  StatusLed::setup();
  Buttons::setup();
  LightSensor::setup();
  Screen::setup();
  WiFiConnection::setup();

  DEBUG_PRINTLN("Application ready !");
}

void loop()
{
  Buttons::loop();
  WiFiConnection::loop();
  SignalStrength::loop();
  NetworkScan::loop();
  LightSensor::loop();
  StatusLed::loop();
  Screen::loop();
  delay(1);
}
