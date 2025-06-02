// WiFiManager_Async.h
// Async WiFi Manager with custom HTML UI for ESP32
#ifndef WIFI_MANAGER_ASYNC_H
#define WIFI_MANAGER_ASYNC_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>

class WiFiManagerAsync {
public:
    WiFiManagerAsync();
    void begin();
    bool isConnected();
    String getLocalIP();
private:
    void startConfigPortal();
    void handleRoot(AsyncWebServerRequest *request);
    void handleSave(AsyncWebServerRequest *request);
    void handleNotFound(AsyncWebServerRequest *request);
    void connectToWiFi(const char* ssid, const char* password);
    void saveCredentials(const String& ssid, const String& password);
    void loadCredentials(String& ssid, String& password);
    AsyncWebServer server;
    DNSServer dns;
    bool connected;
};

#endif // WIFI_MANAGER_ASYNC_H
