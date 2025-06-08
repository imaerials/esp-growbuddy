#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

#define DHTPIN 13 // GPIO04 (D2 on NodeMCU)
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

ESP8266WebServer server(80);

void handleRoot() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang='en'>
    <head>
        <meta charset='UTF-8'>
        <meta name='viewport' content='width=device-width, initial-scale=1.0'>
        <title>Wine Fridge Dashboard</title>
        <style>
            body { font-family: 'Segoe UI', Arial, sans-serif; background: linear-gradient(120deg, #f4f4f9 60%, #ede7f6 100%); color: #222; margin: 0; padding: 0; }
            .dashboard { max-width: 500px; margin: 40px auto; background: #fff; border-radius: 18px; box-shadow: 0 4px 24px rgba(123,31,162,0.10); padding: 2.5em 2em 2em 2em; }
            h1 { color: #7b1fa2; text-align: center; margin-bottom: 2em; letter-spacing: 1px; }
            .cards { display: flex; flex-wrap: wrap; gap: 1.5em; justify-content: center; margin-bottom: 2em; }
            .card {
                flex: 1 1 180px; min-width: 160px; max-width: 220px;
                background: #f8f6fc; border-radius: 12px; box-shadow: 0 1px 8px rgba(123,31,162,0.07);
                padding: 1.2em 1em; display: flex; flex-direction: column; align-items: center; justify-content: center;
            }
            .icon { margin-bottom: 0.5em; }
            .value { color: #7b1fa2; font-size: 2.1em; font-weight: bold; margin-bottom: 0.2em; }
            .label { font-weight: 500; color: #333; font-size: 1.1em; margin-bottom: 0.3em; }
            .badge {
                display: inline-block; background: #7b1fa2; color: #fff; border-radius: 20px; padding: 0.3em 1em; font-size: 1em; font-weight: bold; margin-top: 0.3em;
            }
            .error { color: #c62828; font-weight: bold; text-align: center; margin: 1em 0; }
            .info-list { margin: 0 0 1.5em 0; padding: 0; list-style: none; }
            .info-list li { margin: 0.5em 0; font-size: 1.1em; display: flex; align-items: center; }
            .info-list .info-label { width: 120px; color: #555; font-weight: 500; }
            .info-list .info-value { color: #7b1fa2; font-weight: 600; }
            .wifi-icon { margin-right: 0.5em; }
        </style>
    </head>
    <body>
        <div class='dashboard'>
            <h1>Wine Fridge Dashboard</h1>
            <ul class='info-list'>
                <li><span class='wifi-icon'><svg width='22' height='22' fill='none' stroke='#7b1fa2' stroke-width='2' stroke-linecap='round' stroke-linejoin='round'><path d='M1 10a21 21 0 0 1 22 0'/><path d='M5 14a15 15 0 0 1 14 0'/><path d='M9 18a7 7 0 0 1 6 0'/><circle cx='12' cy='21' r='1'/></svg></span><span class='info-label'>IP Address:</span> <span class='info-value'>%IP%</span></li>
                <li><span class='info-label'>SSID:</span> <span class='info-value'>%SSID%</span></li>
                <li><span class='info-label'>Signal:</span> <span class='badge'>%RSSI% dBm</span></li>
            </ul>
            <div class='cards'>
                %DHTINFO%
            </div>
        </div>
    </body>
    </html>
    )rawliteral";
    String dhtInfo;
    if (isnan(h) || isnan(t)) {
        dhtInfo = "<div class='card error'>DHT11 Sensor: Error reading data</div>";
    } else {
        dhtInfo = "<div class='card'>"
            "<div class='icon'>"
            "<img src='https://img.icons8.com/fluency/48/000000/temperature.png' alt='Temperature' width='48' height='48'>"
            "</div>"
            "<div class='label'>Temperature</div>"
            "<div class='value'>" + String(t, 1) + " &deg;C</div>"
            "<div class='badge'>" + (t < 10 ? "Cold" : t > 20 ? "Warm" : "OK") + "</div>"
            "</div>";
        dhtInfo += "<div class='card'>"
            "<div class='icon'>"
            "<img src='https://img.icons8.com/fluency/48/000000/hygrometer.png' alt='Humidity' width='48' height='48'>"
            "</div>"
            "<div class='label'>Humidity</div>"
            "<div class='value'>" + String(h, 1) + " %</div>"
            "<div class='badge'>" + (h < 40 ? "Dry" : h > 70 ? "Humid" : "OK") + "</div>"
            "</div>";
    }
    html.replace("%IP%", WiFi.localIP().toString());
    html.replace("%SSID%", WiFi.SSID());
    html.replace("%RSSI%", String(WiFi.RSSI()));
    html.replace("%DHTINFO%", dhtInfo);
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
    server.on("/", handleRoot);
    server.begin();
    Serial.println("Web server started. Open http://" + WiFi.localIP().toString());
}

void loop() {
    server.handleClient();
    // Main code here
}