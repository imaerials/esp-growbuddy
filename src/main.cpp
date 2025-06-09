#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <time.h>

#define DHTPIN 13 // GPIO04 (D2 on NodeMCU)
#define DHTTYPE DHT11
#define RELAY_PIN 14 // D5 on NodeMCU
#define RELAY2_PIN 12 // D6 on NodeMCU
DHT dht(DHTPIN, DHTTYPE);
bool relayState = false;
bool relay2State = false;

ESP8266WebServer server(80);

// Function prototype
String getCurrentTimeString();

void handleRelayToggle() {
    relayState = !relayState;
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    server.sendHeader("Location", "/");
    server.send(303); // Redirect to root
}

void handleRelay2Toggle() {
    relay2State = !relay2State;
    digitalWrite(RELAY2_PIN, relay2State ? HIGH : LOW);
    server.sendHeader("Location", "/");
    server.send(303); // Redirect to root
}

void handleStatus() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    String json = "{";
    json += "\"temperature\":" + (isnan(t) ? "null" : String(t, 1)) + ",";
    json += "\"humidity\":" + (isnan(h) ? "null" : String(h, 1)) + ",";
    json += "\"relay1\":" + String(relayState ? 1 : 0) + ",";
    json += "\"relay2\":" + String(relay2State ? 1 : 0) + ",";
    json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
    json += "\"ssid\":\"" + WiFi.SSID() + "\",";
    json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    json += "\"time\":\"" + getCurrentTimeString() + "\"";
    json += "}";
    server.send(200, "application/json", json);
}

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
            .relay-btn { width:100%;padding:1em 0;margin-top:1.5em;font-size:1.2em;background:#7b1fa2;color:#fff;border:none;border-radius:12px;cursor:pointer;transition:background 0.2s; }
            .relay-btn.on { background: #43a047; }
            .relay-btn.off { background: #c62828; }
            .relay-btn:hover { opacity: 0.85; }
        </style>
        <script>
        function updateDashboard(data) {
            if (data.temperature !== null) {
                document.getElementById('temp-value').innerHTML = data.temperature.toFixed(1) + ' &deg;C';
                document.getElementById('temp-badge').innerText = data.temperature < 10 ? 'Cold' : (data.temperature > 20 ? 'Warm' : 'OK');
            } else {
                document.getElementById('temp-value').innerText = 'Error';
                document.getElementById('temp-badge').innerText = '';
            }
            if (data.humidity !== null) {
                document.getElementById('hum-value').innerHTML = data.humidity.toFixed(1) + ' %';
                document.getElementById('hum-badge').innerText = data.humidity < 40 ? 'Dry' : (data.humidity > 70 ? 'Humid' : 'OK');
            } else {
                document.getElementById('hum-value').innerText = 'Error';
                document.getElementById('hum-badge').innerText = '';
            }
            document.getElementById('ip-value').innerText = data.ip;
            document.getElementById('ssid-value').innerText = data.ssid;
            document.getElementById('rssi-value').innerText = data.rssi + ' dBm';
            document.getElementById('time-value').innerText = data.time;
            document.getElementById('relay1-btn').innerText = 'Relay 1: ' + (data.relay1 ? 'ON' : 'OFF');
            document.getElementById('relay1-btn').className = 'relay-btn ' + (data.relay1 ? 'on' : 'off');
            document.getElementById('relay2-btn').innerText = 'Relay 2: ' + (data.relay2 ? 'ON' : 'OFF');
            document.getElementById('relay2-btn').className = 'relay-btn ' + (data.relay2 ? 'on' : 'off');
        }
        function fetchStatus() {
            fetch('/status').then(r => r.json()).then(updateDashboard);
        }
        setInterval(fetchStatus, 3000);
        window.onload = fetchStatus;
        </script>
    </head>
    <body>
        <div class='dashboard'>
            <h1>Wine Fridge Dashboard</h1>
            <ul class='info-list'>
                <li><span class='wifi-icon'><svg width='22' height='22' fill='none' stroke='#7b1fa2' stroke-width='2' stroke-linecap='round' stroke-linejoin='round'><path d='M1 10a21 21 0 0 1 22 0'/><path d='M5 14a15 15 0 0 1 14 0'/><path d='M9 18a7 7 0 0 1 6 0'/><circle cx='12' cy='21' r='1'/></svg></span><span class='info-label'>IP Address:</span> <span class='info-value' id='ip-value'>%IP%</span></li>
                <li><span class='info-label'>SSID:</span> <span class='info-value' id='ssid-value'>%SSID%</span></li>
                <li><span class='info-label'>Signal:</span> <span class='badge' id='rssi-value'>%RSSI% dBm</span></li>
                <li><span class='info-label'>Time:</span> <span class='info-value' id='time-value'>%TIME%</span></li>
            </ul>
            <div class='cards'>
                <div class='card'>
                    <div class='icon'>
                        <img src='https://img.icons8.com/fluency/48/000000/temperature.png' alt='Temperature' width='48' height='48'>
                    </div>
                    <div class='label'>Temperature</div>
                    <div class='value' id='temp-value'>%TEMP%</div>
                    <div class='badge' id='temp-badge'>%TEMPBADGE%</div>
                </div>
                <div class='card'>
                    <div class='icon'>
                        <img src='https://img.icons8.com/fluency/48/000000/hygrometer.png' alt='Humidity' width='48' height='48'>
                    </div>
                    <div class='label'>Humidity</div>
                    <div class='value' id='hum-value'>%HUM%</div>
                    <div class='badge' id='hum-badge'>%HUMBADGE%</div>
                </div>
            </div>
            <form method='POST' action='/relay'>
                <button id='relay1-btn' class='relay-btn %RELAYCLASS%' type='submit'>Relay 1: %RELAYSTATE%</button>
            </form>
            <form method='POST' action='/relay2'>
                <button id='relay2-btn' class='relay-btn %RELAY2CLASS%' type='submit'>Relay 2: %RELAY2STATE%</button>
            </form>
        </div>
    </body>
    </html>
    )rawliteral";
    String tempStr = isnan(t) ? "Error" : String(t, 1) + " &deg;C";
    String tempBadge = isnan(t) ? "" : (t < 10 ? "Cold" : t > 20 ? "Warm" : "OK");
    String humStr = isnan(h) ? "Error" : String(h, 1) + " %";
    String humBadge = isnan(h) ? "" : (h < 40 ? "Dry" : h > 70 ? "Humid" : "OK");
    html.replace("%IP%", WiFi.localIP().toString());
    html.replace("%SSID%", WiFi.SSID());
    html.replace("%RSSI%", String(WiFi.RSSI()));
    html.replace("%TIME%", getCurrentTimeString());
    html.replace("%TEMP%", tempStr);
    html.replace("%TEMPBADGE%", tempBadge);
    html.replace("%HUM%", humStr);
    html.replace("%HUMBADGE%", humBadge);
    html.replace("%RELAYSTATE%", relayState ? "ON" : "OFF");
    html.replace("%RELAYCLASS%", relayState ? "on" : "off");
    html.replace("%RELAY2STATE%", relay2State ? "ON" : "OFF");
    html.replace("%RELAY2CLASS%", relay2State ? "on" : "off");
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
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    pinMode(RELAY2_PIN, OUTPUT);
    digitalWrite(RELAY2_PIN, LOW);
    server.on("/", handleRoot);
    server.on("/relay", HTTP_POST, handleRelayToggle);
    server.on("/relay2", HTTP_POST, handleRelay2Toggle);
    server.on("/status", HTTP_GET, handleStatus);
    server.begin();
    Serial.println("Web server started. Open http://" + WiFi.localIP().toString());

    // NTP time setup for Argentina (UTC-3, no DST)
    configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("Waiting for NTP time sync...");
    time_t now = time(nullptr);
    int retry = 0;
    const int retry_count = 20;
    while (now < 8 * 3600 * 2 && retry < retry_count) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
        retry++;
    }
    Serial.println("");
    if (now < 8 * 3600 * 2) {
        Serial.println("Failed to sync time over NTP");
    } else {
        Serial.println("Time synchronized!");
        struct tm timeinfo;
        gmtime_r(&now, &timeinfo);
        Serial.printf("Current time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }
}

void loop() {
    server.handleClient();
    // Main code here
}

// Helper to get current time as string
String getCurrentTimeString() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    char buf[20];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    return String(buf);
}