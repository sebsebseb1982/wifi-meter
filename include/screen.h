#ifndef SCREEN_H
#define SCREEN_H

#include <Arduino.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_REFRESH_PERIOD_MS 100
// The panel is mounted upside down (0 = normal, 2 = 180 degrees)
#define SCREEN_ROTATION 2
// Bicolor panel: the 16 first physical rows are yellow, the rest is blue.
// Once rotated by 180 degrees the yellow band is at the bottom: y in [SCREEN_YELLOW_TOP, 63]
#define SCREEN_YELLOW_TOP 48
// OLED contrast range driven by the light sensor
#define SCREEN_CONTRAST_MIN 0x10
#define SCREEN_CONTRAST_MAX 0xCF

enum ScreenPage {
  PAGE_SIGNAL,
  PAGE_GRAPH,
  PAGE_INFO,
  PAGE_SCAN,
  PAGE_COUNT
};

class Screen {
  public:
    static void setup();
    static void loop();

  private:
    static boolean available;
    static unsigned long lastRefreshMs;
    static int page;
    static int contrast;
    static void handleButtons();
    static void changePage(int step);
    static void updateContrast();
    static void drawHeader(const char *title);
    static void drawPageDots();
    static void drawPortal();
    static void drawConnecting();
    static void drawSignal();
    static void drawGraph();
    static void drawInfo();
    static void drawScan();
    static void drawBars(int x, int bottom, int bars);
    static void drawHistory(int top, int height);
};

#endif
