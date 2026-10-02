#pragma once

// Uncomment to enable debug traces
//#define DEBUG_SERIAL

#ifdef DEBUG_SERIAL
#include <Arduino.h>
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif
