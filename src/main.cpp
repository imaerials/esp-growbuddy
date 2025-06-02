#include <WiFiManager.h>

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
}

void loop() {
    // Main code here
}