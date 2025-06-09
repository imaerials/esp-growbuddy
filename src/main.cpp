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

// Timezone setting (default: UTC-3 for Mercosur)
int timezoneOffset = -3 * 3600; // seconds
String timezoneName = "America/Argentina/Buenos_Aires";

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
        <title>My Growbuddy</title>
        <style>
            body { font-family: 'Segoe UI', Arial, sans-serif; background: linear-gradient(120deg, #e8f5e9 60%, #c8e6c9 100%); color: #234d20; margin: 0; padding: 0; }
            .topnav {
                width: 100%;
                background: #388e3c;
                color: #fff;
                display: flex;
                align-items: center;
                justify-content: space-between;
                padding: 1em 1.5em;
                box-sizing: border-box;
                position: sticky;
                top: 0;
                z-index: 10;
            }
            .topnav .logo {
                font-size: 1.5em;
                font-weight: bold;
                letter-spacing: 1px;
                display: flex;
                align-items: center;
            }
            .topnav .logo svg {
                margin-right: 0.5em;
            }
            .topnav .menu {
                font-size: 1.1em;
                font-weight: 500;
                display: flex;
                gap: 1.5em;
            }
            .dashboard {
                max-width: 540px;
                margin: 40px auto;
                background: #fff;
                border-radius: 20px;
                box-shadow: 0 4px 24px rgba(56,142,60,0.13);
                padding: 2.5em 2em 2em 2em;
            }
            h1 { display:none; }
            .cards {
                display: flex;
                flex-wrap: wrap;
                gap: 1.5em;
                justify-content: center;
                margin-bottom: 2em;
            }
            .card {
                flex: 1 1 180px;
                min-width: 140px;
                max-width: 220px;
                background: #f1f8e9;
                border-radius: 14px;
                box-shadow: 0 1px 8px rgba(56,142,60,0.09);
                padding: 1.2em 1em;
                display: flex;
                flex-direction: column;
                align-items: center;
                justify-content: center;
            }
            .icon { margin-bottom: 0.5em; }
            .value { color: #388e3c; font-size: 2.1em; font-weight: bold; margin-bottom: 0.2em; }
            .label { font-weight: 500; color: #234d20; font-size: 1.1em; margin-bottom: 0.3em; }
            .badge {
                display: inline-block;
                background: #388e3c;
                color: #fff;
                border-radius: 20px;
                padding: 0.3em 1em;
                font-size: 1em;
                font-weight: bold;
                margin-top: 0.3em;
            }
            .error { color: #c62828; font-weight: bold; text-align: center; margin: 1em 0; }
            .info-list {
                margin: 0 0 1.5em 0;
                padding: 0;
                list-style: none;
                display: flex;
                flex-wrap: wrap;
                gap: 1em 2em;
                justify-content: center;
            }
            .info-list li {
                margin: 0.5em 0;
                font-size: 1.1em;
                display: flex;
                align-items: center;
                min-width: 140px;
            }
            .info-list .info-label { width: 90px; color: #388e3c; font-weight: 500; }
            .info-list .info-value { color: #234d20; font-weight: 600; }
            .wifi-icon { margin-right: 0.5em; }
            .controls {
                display: flex;
                flex-direction: column;
                gap: 10px;
                margin-bottom: 1.5em;
            }
            .device {
                display: flex;
                justify-content: space-between;
                align-items: center;
                background: #e8f5e9;
                color: #234d20;
                padding: 10px 16px;
                border-radius: 8px;
                font-size: 1.1em;
                margin-bottom: 0;
                box-shadow: 0 1px 4px rgba(56,142,60,0.07);
            }
            .btn {
                padding: 6px 18px;
                font-weight: bold;
                border: none;
                border-radius: 6px;
                cursor: pointer;
                background: #666;
                color: #fff;
                font-size: 1em;
                transition: background 0.2s;
            }
            .btn.on {
                background: #2ecc71;
            }
            .btn.off {
                background: #e74c3c;
            }
            @media (max-width: 700px) {
                .dashboard { max-width: 98vw; padding: 1.2em 0.5em; }
                .cards { flex-direction: column; gap: 1em; }
                .card { min-width: 0; width: 100%; }
                .info-list { flex-direction: column; gap: 0.5em; }
            }
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
            // Fix relay button text and classes for new .btn style
            var relay1Btn = document.getElementById('relay1-btn');
            relay1Btn.textContent = data.relay1 ? 'ON' : 'OFF';
            relay1Btn.className = 'btn ' + (data.relay1 ? 'on' : 'off');
            var relay2Btn = document.getElementById('relay2-btn');
            relay2Btn.textContent = data.relay2 ? 'ON' : 'OFF';
            relay2Btn.className = 'btn ' + (data.relay2 ? 'on' : 'off');
        }
        function fetchStatus() {
            fetch('/status').then(r => r.json()).then(updateDashboard);
        }
        setInterval(fetchStatus, 3000);
        window.onload = fetchStatus;
        </script>
    </head>
    <body>
        <div class='topnav'>
            <span class='logo'>
                <svg width='28' height='28' viewBox='0 0 24 24' fill='none' stroke='#fff' stroke-width='2' stroke-linecap='round' stroke-linejoin='round'><path d='M12 2v20M2 12h20'/><circle cx='12' cy='12' r='10' stroke='#fff' stroke-width='2' fill='#66bb6a'/></svg>
                My Growbuddy
            </span>
            <span class='menu'>
                <a href='/' style='color:inherit;text-decoration:none;margin-right:1em;'>Dashboard</a>
                <a href='/settings' style='color:inherit;text-decoration:none;margin-right:1em;'>Settings</a>
                <span>About</span>
            </span>
        </div>
        <div class='dashboard'>
            <div style='text-align:center;margin-bottom:2em;'>
                <img src="https://img.icons8.com/color/96/000000/plant-under-sun.png" alt="Growbuddy Plant" style="width:64px;height:64px;vertical-align:middle;">
                <div style='font-size:1.3em;font-weight:500;margin-top:0.5em;color:#388e3c;'>Greenhouse/Indoor Garden Monitor</div>
            </div>
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
            <div class='controls' style='margin:2em 0;'>
                <div class='device'>
                    <span>💡 Light</span>
                    <form method='POST' action='/relay' style='margin:0;'>
                        <button id='relay1-btn' class='btn %RELAYCLASS%' type='submit'>%RELAYSTATE%</button>
                    </form>
                </div>
                <div class='device'>
                    <span>🌬️ Fan</span>
                    <form method='POST' action='/relay2' style='margin:0;'>
                        <button id='relay2-btn' class='btn %RELAY2CLASS%' type='submit'>%RELAY2STATE%</button>
                    </form>
                </div>
            </div>
            <ul class='info-list'>
                <li><span class='wifi-icon'><svg width='22' height='22' fill='none' stroke='#7b1fa2' stroke-width='2' stroke-linecap='round' stroke-linejoin='round'><path d='M1 10a21 21 0 0 1 22 0'/><path d='M5 14a15 15 0 0 1 14 0'/><path d='M9 18a7 7 0 0 1 6 0'/><circle cx='12' cy='21' r='1'/></svg></span><span class='info-label'>IP:</span> <span class='info-value' id='ip-value'>%IP%</span></li>
                <li><span class='info-label'>SSID:</span> <span class='info-value' id='ssid-value'>%SSID%</span></li>
                <li><span class='info-label'>Signal:</span> <span class='badge' id='rssi-value'>%RSSI% dBm</span></li>
                <li><span class='info-label'>Time:</span> <span class='info-value' id='time-value'>%TIME%</span></li>
            </ul>
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
    String relay1Color = relayState ? "#43a047" : "#c62828";
    String relay2Color = relay2State ? "#43a047" : "#c62828";
    String relay1Label = relayState ? "ON" : "OFF";
    String relay2Label = relay2State ? "ON" : "OFF";
    html.replace("%RELAY1COLOR%", relay1Color);
    html.replace("%RELAY2COLOR%", relay2Color);
    html.replace("%RELAY1LABEL%", relay1Label);
    html.replace("%RELAY2LABEL%", relay2Label);
    server.send(200, "text/html", html);
}

void handleSettings() {
    String html = R"rawliteral(
    <!DOCTYPE html>
    <html lang='en'>
    <head>
        <meta charset='UTF-8'>
        <meta name='viewport' content='width=device-width, initial-scale=1.0'>
        <title>Settings - My Growbuddy</title>
        <style>
            body { font-family: 'Segoe UI', Arial, sans-serif; background: linear-gradient(120deg, #e8f5e9 60%, #c8e6c9 100%); color: #234d20; margin: 0; padding: 0; }
            .topnav {
                width: 100%;
                background: #388e3c;
                color: #fff;
                display: flex;
                align-items: center;
                justify-content: space-between;
                padding: 1em 1.5em;
                box-sizing: border-box;
                position: sticky;
                top: 0;
                z-index: 10;
            }
            .topnav .logo {
                font-size: 1.5em;
                font-weight: bold;
                letter-spacing: 1px;
                display: flex;
                align-items: center;
            }
            .topnav .logo svg {
                margin-right: 0.5em;
            }
            .topnav .menu {
                font-size: 1.1em;
                font-weight: 500;
                display: flex;
                gap: 1.5em;
            }
            .topnav .menu a {
                color: inherit;
                text-decoration: none;
                margin-right: 1em;
            }
            .settings-container { max-width: 400px; margin: 40px auto; background: #fff; border-radius: 16px; box-shadow: 0 4px 24px rgba(56,142,60,0.13); padding: 2em; }
            h2 { color: #388e3c; text-align: center; margin-bottom: 1.5em; }
            label { display: block; margin-bottom: 0.5em; font-weight: 500; }
            select, button { width: 100%; padding: 0.7em; border-radius: 8px; border: 1px solid #bdbdbd; margin-bottom: 1.5em; font-size: 1em; }
            button { background: #388e3c; color: #fff; border: none; font-weight: bold; cursor: pointer; transition: background 0.2s; }
            button:hover { background: #2e7031; }
        </style>
    </head>
    <body>
        <div class='topnav'>
            <span class='logo'>
                <svg width='28' height='28' viewBox='0 0 24 24' fill='none' stroke='#fff' stroke-width='2' stroke-linecap='round' stroke-linejoin='round'><path d='M12 2v20M2 12h20'/><circle cx='12' cy='12' r='10' stroke='#fff' stroke-width='2' fill='#66bb6a'/></svg>
                My Growbuddy
            </span>
            <span class='menu'>
                <a href='/' style='color:inherit;text-decoration:none;margin-right:1em;'>Dashboard</a>
                <a href='/settings' style='color:inherit;text-decoration:none;margin-right:1em;'>Settings</a>
                <span>About</span>
            </span>
        </div>
        <div class='settings-container'>
            <h2>Settings</h2>
            <form method='POST' action='/settings'>
                <label for='timezone'>Time Zone (Mercosur):</label>
                <select id='timezone' name='timezone'>
                    <option value='America/Argentina/Buenos_Aires'" + (timezoneName == "America/Argentina/Buenos_Aires" ? " selected" : "") + ">Argentina (UTC-3)</option>
                    <option value='America/Sao_Paulo'" + (timezoneName == "America/Sao_Paulo" ? " selected" : "") + ">Brazil (UTC-3)</option>
                    <option value='America/Montevideo'" + (timezoneName == "America/Montevideo" ? " selected" : "") + ">Uruguay (UTC-3)</option>
                    <option value='America/Asuncion'" + (timezoneName == "America/Asuncion" ? " selected" : "") + ">Paraguay (UTC-4/UTC-3 DST)</option>
                </select>
                <button type='submit'>Save</button>
            </form>
        </div>
    </body>
    </html>
    )rawliteral";
    server.send(200, "text/html", html);
}

void handleSettingsPost() {
    if (server.hasArg("timezone")) {
        timezoneName = server.arg("timezone");
        if (timezoneName == "America/Argentina/Buenos_Aires" || timezoneName == "America/Sao_Paulo" || timezoneName == "America/Montevideo") {
            timezoneOffset = -3 * 3600;
        } else if (timezoneName == "America/Asuncion") {
            timezoneOffset = -4 * 3600; // Paraguay standard, DST not handled
        }
        configTime(timezoneOffset, 0, "pool.ntp.org", "time.nist.gov");
    }
    server.sendHeader("Location", "/settings");
    server.send(303);
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
    server.on("/settings", HTTP_GET, handleSettings);
    server.on("/settings", HTTP_POST, handleSettingsPost);
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