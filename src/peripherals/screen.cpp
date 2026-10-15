#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "screen.h"
#include "pinout.h"
#include "debug.h"
#include "buttons.h"
#include "light-sensor.h"
#include "network-scan.h"
#include "signal-strength.h"
#include "status-led.h"
#include "wifi-portal.h"
#include "wifi-connection.h"

static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

boolean Screen::available = false;
unsigned long Screen::lastRefreshMs = 0;
int Screen::page = PAGE_SIGNAL;
int Screen::contrast = SCREEN_CONTRAST_MAX;

void Screen::setup() {
  DEBUG_PRINTLN("Screen::setup()");
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    DEBUG_PRINTLN("SSD1306 not found");
    return;
  }
  available = true;

  display.setRotation(SCREEN_ROTATION);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(10, 6);
  display.print("WiFi");
  display.setCursor(10, 26);
  display.print("meter");
  display.setTextSize(1);
  display.setCursor(10, SCREEN_YELLOW_TOP + 5);
  display.print("RSSI en temps reel");
  display.display();
}

void Screen::loop() {
  handleButtons();
  if (!available || millis() - lastRefreshMs < SCREEN_REFRESH_PERIOD_MS) {
    return;
  }
  lastRefreshMs = millis();

  updateContrast();
  display.clearDisplay();
  if (WiFiConnection::isPortal()) {
    drawPortal();
  } else if (page == PAGE_SCAN) {
    drawScan();
  } else if (!WiFiConnection::isConnected() || !SignalStrength::hasValue()) {
    drawConnecting();
  } else if (page == PAGE_GRAPH) {
    drawGraph();
  } else if (page == PAGE_INFO) {
    drawInfo();
  } else {
    drawSignal();
  }
  display.display();
}

// Left / right: previous / next page. Middle: action of the current page,
// or a long press to open the WiFi configuration portal.
void Screen::handleButtons() {
  if (Buttons::longPressed(BUTTON_MIDDLE) && !WiFiConnection::isPortal()) {
    WiFiConnection::startPortal();
  }
  if (WiFiConnection::isPortal()) {
    // Nothing to navigate while the portal is open
    Buttons::pressed(BUTTON_LEFT);
    Buttons::pressed(BUTTON_MIDDLE);
    Buttons::pressed(BUTTON_RIGHT);
    return;
  }
  if (Buttons::pressed(BUTTON_LEFT)) {
    changePage(-1);
  }
  if (Buttons::pressed(BUTTON_RIGHT)) {
    changePage(1);
  }
  if (Buttons::pressed(BUTTON_MIDDLE)) {
    switch (page) {
      case PAGE_SIGNAL: StatusLed::toggleMute(); break;
      case PAGE_GRAPH:  SignalStrength::resetStats(); break;
      case PAGE_SCAN:   NetworkScan::start(); break;
    }
  }
}

void Screen::changePage(int step) {
  page = (page + step + PAGE_COUNT) % PAGE_COUNT;
  if (page == PAGE_SCAN && !NetworkScan::hasResults()) {
    NetworkScan::start();
  }
}

// Dim the OLED in the dark (also saves the panel)
void Screen::updateContrast() {
  if (!available || !LightSensor::available()) {
    return;
  }
  int target = map(LightSensor::level(), 0, 100, SCREEN_CONTRAST_MIN, SCREEN_CONTRAST_MAX);
  // Small hysteresis: avoid flooding the I2C bus for nothing
  if (abs(target - contrast) < 6) {
    return;
  }
  contrast = target;
  display.ssd1306_command(SSD1306_SETCONTRAST);
  display.ssd1306_command(contrast);
}

void Screen::drawPageDots() {
  for (int i = 0; i < PAGE_COUNT; i++) {
    int x = SCREEN_WIDTH - (PAGE_COUNT - i) * 6;
    if (i == page) {
      display.fillRect(x, 0, 4, 4, SSD1306_WHITE);
    } else {
      display.drawRect(x, 0, 4, 4, SSD1306_WHITE);
    }
  }
}

void Screen::drawHeader(const char *title) {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(title);
  drawPageDots();
  display.drawFastHLine(0, 9, SCREEN_WIDTH, SSD1306_WHITE);
}

void Screen::drawPortal() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Configuration WiFi");
  display.drawFastHLine(0, 9, SCREEN_WIDTH, SSD1306_WHITE);
  display.setCursor(0, 13);
  display.print("1 Rejoindre le WiFi");
  display.setCursor(12, 23);
  display.print(WiFiPortal::apName());
  display.setCursor(0, 34);
  display.printf("2 Ouvrir %s", WiFiPortal::apAddress().c_str());

  // Yellow band: connected devices
  display.setCursor(0, SCREEN_YELLOW_TOP + 5);
  display.printf("Appareils : %d", WiFiPortal::clientCount());
}

void Screen::drawConnecting() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Connexion a :");
  drawPageDots();
  display.setCursor(0, 12);
  display.print(String(WiFiConnection::ssid()).substring(0, 21));

  display.setCursor(0, 32);
  display.print(WiFiConnection::statusText());
  display.setCursor(0, 40);
  display.printf("Essai %d/%d", WiFiConnection::attempt(), WIFI_MAX_ATTEMPTS);

  unsigned long elapsed = WiFiConnection::connectingSinceMs();
  // Progress in the yellow band
  display.setCursor(0, SCREEN_YELLOW_TOP + 5);
  for (int i = 0; i < (int)(elapsed / 300) % 4; i++) {
    display.print('.');
  }
  display.setCursor(100, SCREEN_YELLOW_TOP + 5);
  display.printf("%2lus", elapsed / 1000);
}

void Screen::drawSignal() {
  // Header: SSID, channel and IP
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(WiFi.SSID().substring(0, 17));
  drawPageDots();
  display.setCursor(0, 8);
  display.printf("ch%d %s", WiFi.channel(), WiFi.localIP().toString().c_str());
  if (StatusLed::isMuted()) {
    display.setCursor(104, 8);
    display.print("mute");
  }

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

  // History in the yellow band
  drawHistory(SCREEN_YELLOW_TOP + 2, SCREEN_HEIGHT - SCREEN_YELLOW_TOP - 2);
}

void Screen::drawGraph() {
  drawHeader("Historique");

  display.setCursor(0, 12);
  display.printf("min%d max%d moy%d", SignalStrength::minRssi(), SignalStrength::maxRssi(), SignalStrength::averageRssi());

  // Full height graph: the yellow band at the bottom is the weak signal area.
  // Dotted guide lines at 25 / 50 / 75 % quality
  const int top = 22;
  const int height = SCREEN_HEIGHT - top;
  for (int q = 25; q < 100; q += 25) {
    int y = top + height - 1 - (q * (height - 1)) / 100;
    for (int x = 0; x < SCREEN_WIDTH; x += 4) {
      display.drawPixel(x, y, SSD1306_WHITE);
    }
  }
  drawHistory(top, height);
}

void Screen::drawInfo() {
  drawHeader("Infos");

  display.setCursor(0, 12);
  display.printf("IP %s", WiFi.localIP().toString().c_str());
  display.setCursor(0, 21);
  display.printf("GW %s", WiFi.gatewayIP().toString().c_str());
  display.setCursor(0, 30);
  display.printf("MAC %s", WiFi.macAddress().c_str());
  display.setCursor(0, 39);
  display.printf("AP  %s", WiFi.BSSIDstr().c_str());

  unsigned long seconds = millis() / 1000;
  // Yellow band: uptime and light
  display.setCursor(0, SCREEN_YELLOW_TOP + 1);
  display.printf("Up %02luh%02lum  Conn %u", seconds / 3600, (seconds / 60) % 60, WiFiConnection::nbConnection);

  display.setCursor(0, SCREEN_YELLOW_TOP + 9);
  if (LightSensor::available()) {
    display.printf("Lum %d%% Ecran %d%%", LightSensor::level(), (contrast * 100) / 255);
  } else {
    display.print("Capteur lum. HS");
  }
}

void Screen::drawScan() {
  char title[24];
  if (NetworkScan::isScanning()) {
    snprintf(title, sizeof(title), "Scan...");
  } else {
    snprintf(title, sizeof(title), "Reseaux (%d)", NetworkScan::count());
  }
  drawHeader(title);

  if (!NetworkScan::hasResults()) {
    display.setCursor(0, 28);
    display.print("Recherche en cours");
    return;
  }

  String connected = WiFi.SSID();
  for (int i = 0; i < NetworkScan::count(); i++) {
    const ScanResult &network = NetworkScan::result(i);
    // 4 strongest networks in the blue area, the others in the yellow band
    int y = i < 4 ? 12 + i * 9 : SCREEN_YELLOW_TOP + 1 + (i - 4) * 8;
    display.setCursor(0, y);
    display.printf("%c%-12.12s %2d %4d",
                   WiFiConnection::isConnected() && connected == network.ssid ? '*' : ' ',
                   network.ssid, network.channel, network.rssi);
  }
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
