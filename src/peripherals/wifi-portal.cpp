#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include "wifi-portal.h"
#include "wifi-connection.h"
#include "network-scan.h"
#include "debug.h"

#define DNS_PORT 53
// Delay before the restart that applies the new credentials, so the answer reaches the browser
#define RESTART_DELAY_MS 2000

struct PortalNetwork {
  String ssid;
  int rssi;
  boolean secured;
};

static WebServer server(80);
static DNSServer dns;
static PortalNetwork networks[WIFI_PORTAL_MAX_NETWORKS];
static int networkCount = 0;
static boolean running = false;
static unsigned long restartAtMs = 0;

static const char PAGE_HEAD[] PROGMEM =
  "<!DOCTYPE html><html lang=fr><head><meta charset=utf-8>"
  "<meta name=viewport content='width=device-width,initial-scale=1'>"
  "<title>WiFi meter</title><style>"
  "body{font-family:sans-serif;margin:0;background:#101820;color:#eee}"
  "main{max-width:420px;margin:auto;padding:16px}"
  "h1{font-size:1.4em}"
  "a.net{display:flex;justify-content:space-between;padding:12px;margin:6px 0;"
  "background:#1c2a38;border-radius:8px;color:#eee;text-decoration:none}"
  "input,button{width:100%;box-sizing:border-box;padding:12px;margin:6px 0;"
  "border-radius:8px;border:0;font-size:1em}"
  "button{background:#1e90ff;color:#fff}"
  ".sec{background:none;color:#8ab;border:1px solid #345}"
  "</style></head><body><main>";

static const char PAGE_FOOT[] PROGMEM = "</main></body></html>";

static String htmlEscape(const String &text) {
  String out;
  out.reserve(text.length() + 8);
  for (size_t i = 0; i < text.length(); i++) {
    char c = text[i];
    switch (c) {
      case '&':  out += "&amp;"; break;
      case '<':  out += "&lt;"; break;
      case '>':  out += "&gt;"; break;
      case '"':  out += "&quot;"; break;
      case '\'': out += "&#39;"; break;
      default:   out += c;
    }
  }
  return out;
}

// Blocking scan, sorted by decreasing RSSI, without duplicates or hidden networks
static void scanNetworks() {
  if (NetworkScan::isScanning()) {
    return;
  }
  int found = WiFi.scanNetworks();
  networkCount = 0;
  for (int i = 0; i < found; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) {
      continue;
    }
    int existing = -1;
    for (int j = 0; j < networkCount; j++) {
      if (networks[j].ssid == ssid) {
        existing = j;
      }
    }
    int rssi = WiFi.RSSI(i);
    if (existing >= 0) {
      networks[existing].rssi = max(networks[existing].rssi, rssi);
      continue;
    }
    if (networkCount < WIFI_PORTAL_MAX_NETWORKS) {
      networks[networkCount++] = {ssid, rssi, WiFi.encryptionType(i) != WIFI_AUTH_OPEN};
    }
  }
  WiFi.scanDelete();

  for (int i = 1; i < networkCount; i++) {
    PortalNetwork current = networks[i];
    int j = i - 1;
    while (j >= 0 && networks[j].rssi < current.rssi) {
      networks[j + 1] = networks[j];
      j--;
    }
    networks[j + 1] = current;
  }
}

static void redirectToPortal() {
  server.sendHeader("Location", String("http://") + WiFiPortal::apAddress() + "/", true);
  server.send(302, "text/plain", "");
}

static void handleRoot() {
  String html = FPSTR(PAGE_HEAD);
  html += "<h1>Configurer le WiFi</h1><p>Choisissez le r&eacute;seau &agrave; mesurer :</p>";
  for (int i = 0; i < networkCount; i++) {
    String ssid = htmlEscape(networks[i].ssid);
    html += "<a class=net href='#' data-ssid=\"" + ssid + "\"><span>" + ssid + (networks[i].secured ? " &#128274;" : "") +
            "</span><span>" + String(networks[i].rssi) + " dBm</span></a>";
  }
  if (networkCount == 0) {
    html += "<p>Aucun r&eacute;seau trouv&eacute;.</p>";
  }
  html += "<form method=post action=/save>"
          "<input id=ssid name=ssid placeholder='Nom du r&eacute;seau (SSID)' maxlength=32 required>"
          "<input name=password type=password placeholder='Mot de passe' maxlength=64>"
          "<button>Enregistrer et se connecter</button></form>"
          "<form action=/scan><button class=sec>Rechercher &agrave; nouveau</button></form>"
          "<script>document.querySelectorAll('a.net').forEach(function(a){"
          "a.onclick=function(e){e.preventDefault();document.getElementById('ssid').value=a.dataset.ssid;"
          "document.getElementsByName('password')[0].focus();};});</script>";
  html += FPSTR(PAGE_FOOT);
  server.send(200, "text/html", html);
}

static void handleScan() {
  scanNetworks();
  redirectToPortal();
}

static void handleSave() {
  String ssid = server.arg("ssid");
  ssid.trim();
  if (ssid.length() == 0 || ssid.length() > 32) {
    server.send(400, "text/plain", "SSID invalide");
    return;
  }
  WiFiConnection::saveCredentials(ssid, server.arg("password"));

  String html = FPSTR(PAGE_HEAD);
  html += "<h1>Enregistr&eacute;</h1><p>Le WiFi meter se connecte &agrave; <b>" + htmlEscape(ssid) +
          "</b> et red&eacute;marre. Ce point d'acc&egrave;s va dispara&icirc;tre : reconnectez votre appareil &agrave; "
          "votre r&eacute;seau habituel.</p><p>Si la connexion &eacute;choue, le portail se rouvre "
          "au bout de quelques essais.</p>";
  html += FPSTR(PAGE_FOOT);
  server.send(200, "text/html", html);
  restartAtMs = millis() + RESTART_DELAY_MS;
}

void WiFiPortal::start() {
  if (running) {
    return;
  }
  const char *password = WIFI_PORTAL_AP_PASSWORD;
  WiFi.softAP(WIFI_PORTAL_AP_NAME, password[0] ? password : nullptr);
  delay(100);

  scanNetworks();

  // Every DNS name resolves to the portal: this is what triggers the "sign in" popup of phones
  dns.start(DNS_PORT, "*", WiFi.softAPIP());
  server.on("/", HTTP_GET, handleRoot);
  server.on("/scan", HTTP_GET, handleScan);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(redirectToPortal);
  server.begin();
  running = true;
  restartAtMs = 0;
  DEBUG_PRINT("Portal started on ");
  DEBUG_PRINTLN(WiFi.softAPIP());
}

void WiFiPortal::stop() {
  if (!running) {
    return;
  }
  server.stop();
  dns.stop();
  WiFi.softAPdisconnect(true);
  running = false;
}

void WiFiPortal::loop() {
  if (!running) {
    return;
  }
  dns.processNextRequest();
  server.handleClient();
  if (restartAtMs && millis() >= restartAtMs) {
    ESP.restart();
  }
}

int WiFiPortal::clientCount() {
  return running ? WiFi.softAPgetStationNum() : 0;
}

const char *WiFiPortal::apName() {
  return WIFI_PORTAL_AP_NAME;
}

String WiFiPortal::apAddress() {
  return WiFi.softAPIP().toString();
}
