#ifndef SCREEN_H
#define SCREEN_H

#include <Arduino.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_REFRESH_PERIOD_MS 100

class Screen {
  public:
    static void setup();
    static void loop();

  private:
    static boolean available;
    static unsigned long lastRefreshMs;
    static void drawConnecting();
    static void drawSignal();
    static void drawBars(int x, int bottom, int bars);
    static void drawHistory(int top, int height);
};

#endif
