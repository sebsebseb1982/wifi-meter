#include <Arduino.h>

#include "debug.h"
#include "flash-led.h"
#include "screen.h"
#include "signal-strength.h"
#include "wifi-connection.h"

void setup()
{
  Serial.begin(115200);
  DEBUG_PRINTLN("setup()");

  FlashLed::setup();
  Screen::setup();
  WiFiConnection::setup();

  DEBUG_PRINTLN("Application ready !");
}

void loop()
{
  WiFiConnection::loop();
  SignalStrength::loop();
  FlashLed::loop();
  Screen::loop();
  delay(1);
}
