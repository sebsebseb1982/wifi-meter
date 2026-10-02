#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "screen.h"
#include "pinout.h"
#include "debug.h"
#include "secrets.h"
#include "signal-strength.h"
#include "wifi-connection.h"

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

boolean Screen::available = false;
unsigned long Screen::lastRefreshMs = 0;

void Screen::setup() {
  DEBUG_PRINTLN("Screen::setup()");
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    DEBUG_PRINTLN("SSD1306 not found");
    return;
  }
  available = true;

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(10, 16);
  display.print("WiFi");
  display.setCursor(10, 36);
  display.print("meter");
  display.display();
}

void Screen::loop() {
  if (!available || millis() - lastRefreshMs < SCREEN_REFRESH_PERIOD_MS) {
    return;
  }
  lastRefreshMs = millis();

  display.clearDisplay();
  if (WiFiConnection::isConnected() && SignalStrength::hasValue()) {
    drawSignal();
  } else {
    drawConnecting();
  }
  display.display();
}

void Screen::drawConnecting() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Connexion a :");
  display.setCursor(0, 12);
  display.print(WIFI_SSID);

  display.setCursor(0, 32);
  display.print(WiFiConnection::statusText());

  unsigned long elapsed = WiFiConnection::connectingSinceMs();
  display.setCursor(0, 48);
  for (int i = 0; i < (int)(elapsed / 300) % 4; i++) {
    display.print('.');
  }
  display.setCursor(100, 48);
  display.printf("%2lus", elapsed / 1000);
}

void Screen::drawSignal() {
  // Header: SSID, channel and IP
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(WiFi.SSID().substring(0, 21));
  display.setCursor(0, 8);
  display.printf("ch%d %s", WiFi.channel(), WiFi.localIP().toString().c_str());

  // Big RSSI value
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print(SignalStrength::rssi());
  display.setTextSize(1);
  display.setCursor(display.getCursorX() + 3, 27);
  display.print("dBm");

  drawBars(98, 37, SignalStrength::bars());

  // Quality gauge
  int quality = SignalStrength::quality();
  display.drawRect(0, 40, 100, 7, SSD1306_WHITE);
  display.fillRect(2, 42, (96 * quality) / 100, 3, SSD1306_WHITE);
  display.setCursor(104, 40);
  display.printf("%3d%%", quality);

  drawHistory(50, 14);
}

void Screen::drawBars(int x, int bottom, int bars) {
  for (int i = 0; i < 5; i++) {
    int height = 4 * (i + 1);
    int left = x + i * 6;
    if (i < bars) {
      display.fillRect(left, bottom - height + 1, 4, height, SSD1306_WHITE);
    } else {
      display.drawRect(left, bottom - height + 1, 4, height, SSD1306_WHITE);
    }
  }
}

void Screen::drawHistory(int top, int height) {
  int count = SignalStrength::historyCount();
  int bottom = top + height - 1;
  int offset = SCREEN_WIDTH - count;
  int previousY = -1;
  for (int i = 0; i < count; i++) {
    int y = bottom - (SignalStrength::history(i) * (height - 1)) / 100;
    if (previousY < 0) {
      display.drawPixel(offset + i, y, SSD1306_WHITE);
    } else {
      display.drawLine(offset + i - 1, previousY, offset + i, y, SSD1306_WHITE);
    }
    previousY = y;
  }
}
