#include "buttons.h"
#include "pinout.h"

static const uint8_t BUTTON_PINS[BUTTON_COUNT] = {BUTTON_LEFT_PIN, BUTTON_MIDDLE_PIN, BUTTON_RIGHT_PIN};

boolean Buttons::stable[BUTTON_COUNT] = {false, false, false};
boolean Buttons::lastRaw[BUTTON_COUNT] = {false, false, false};
boolean Buttons::pending[BUTTON_COUNT] = {false, false, false};
boolean Buttons::longPending[BUTTON_COUNT] = {false, false, false};
boolean Buttons::longFired[BUTTON_COUNT] = {false, false, false};
unsigned long Buttons::changedMs[BUTTON_COUNT] = {0, 0, 0};
unsigned long Buttons::pressedMs[BUTTON_COUNT] = {0, 0, 0};

void Buttons::setup() {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    // Buttons are active HIGH: the internal pull-down only backs up the PCB one
    pinMode(BUTTON_PINS[i], INPUT_PULLDOWN);
  }
}

void Buttons::loop() {
  unsigned long now = millis();
  for (int i = 0; i < BUTTON_COUNT; i++) {
    boolean raw = digitalRead(BUTTON_PINS[i]) == HIGH;
    if (raw != lastRaw[i]) {
      lastRaw[i] = raw;
      changedMs[i] = now;
    } else if (raw != stable[i] && now - changedMs[i] >= BUTTON_DEBOUNCE_MS) {
      stable[i] = raw;
      if (raw) {
        pressedMs[i] = now;
        longFired[i] = false;
      } else if (!longFired[i]) {
        pending[i] = true;
      }
    }

    if (stable[i] && !longFired[i] && now - pressedMs[i] >= BUTTON_LONG_PRESS_MS) {
      longFired[i] = true;
      longPending[i] = true;
    }
  }
}

boolean Buttons::pressed(ButtonId button) {
  boolean result = pending[button];
  pending[button] = false;
  return result;
}

boolean Buttons::longPressed(ButtonId button) {
  boolean result = longPending[button];
  longPending[button] = false;
  return result;
}
