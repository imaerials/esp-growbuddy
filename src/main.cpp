#include <WiFiManager.h>
#include <WebServer.h> // For ESP32
#include <DHT.h>

#define DHTPIN 4 // GPIO04
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

WebServer server(80); // Create web server on port 80

void handleRoot() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    String html = "<html><head><title>Wine Fridge Info</title></head><body>";
    html += "<h1>Wine Fridge Device Info</h1>";
    html += "<p><b>IP Address:</b> " + WiFi.localIP().toString() + "</p>";
    html += "<p><b>SSID:</b> " + WiFi.SSID() + "</p>";
    html += "<p><b>Signal Strength (RSSI):</b> " + String(WiFi.RSSI()) + " dBm</p>";
    if (isnan(h) || isnan(t)) {
        html += "<p><b>DHT11 Sensor:</b> Error reading data</p>";
    } else {
        html += "<p><b>Temperature:</b> " + String(t) + " &deg;C</p>";
        html += "<p><b>Humidity:</b> " + String(h) + " %</p>";
    }
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_STA); // Station mode
    WiFiManager wm;
    // Uncomment the next line to reset saved credentials
    // wm.resetSettings();
    bool res = wm.autoConnect("WineFridge-AP");
    if (!res) {
        Serial.println("Failed to connect. Restarting...");
        delay(3000);
        ESP.restart();
    } else {
        Serial.println("Connected!");
        Serial.print("Local IP: ");
        Serial.println(WiFi.localIP());
    }
    dht.begin();
    // Start web server after WiFi is connected
    server.on("/", handleRoot);
    server.begin();
    Serial.println("Web server started. Open http://" + WiFi.localIP().toString());
}

void loop() {
    server.handleClient(); // Handle web server requests
    // Main code here
}