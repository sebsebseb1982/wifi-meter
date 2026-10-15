#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

#define BUTTON_DEBOUNCE_MS 30
#define BUTTON_LONG_PRESS_MS 3000

enum ButtonId {
  BUTTON_LEFT,
  BUTTON_MIDDLE,
  BUTTON_RIGHT,
  BUTTON_COUNT
};

class Buttons {
  public:
    static void setup();
    static void loop();
    // True once per short press, reported on release (the press is consumed by the call)
    static boolean pressed(ButtonId button);
    // True once per press held for BUTTON_LONG_PRESS_MS (the press is consumed by the call)
    static boolean longPressed(ButtonId button);

  private:
    static boolean stable[BUTTON_COUNT];
    static boolean lastRaw[BUTTON_COUNT];
    static boolean pending[BUTTON_COUNT];
    static boolean longPending[BUTTON_COUNT];
    static boolean longFired[BUTTON_COUNT];
    static unsigned long changedMs[BUTTON_COUNT];
    static unsigned long pressedMs[BUTTON_COUNT];
};

#endif
