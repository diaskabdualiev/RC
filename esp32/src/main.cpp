#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <WebServer.h>
#include <ESPmDNS.h> // Для mDNS

// Пины светодиодов
#define LED_PIN_1 25
#define LED_PIN_2 32
#define LED_PIN_3 33

// Начальные данные для Wi-Fi (по умолчанию)
#define DEFAULT_SSID "YourDefaultSSID"
#define DEFAULT_PASSWORD "YourDefaultPassword"

// Флаги постоянного включения
bool led1PermanentOn = false;
bool led2PermanentOn = false;
bool led3PermanentOn = false;

// Флаги для кратковременного включения (1 сек)
bool led1Blinking = false;
bool led2Blinking = false;
bool led3Blinking = false;

// Время начала мигания
unsigned long led1StartTime = 0;
unsigned long led2StartTime = 0;
unsigned long led3StartTime = 0;

// Интервал мигания (1 сек)
const unsigned long blinkInterval = 1000; 

Preferences preferences;
WebServer server(80);
bool wifiConnected = false;

void handleRoot();
void handleSave();
void startWebServer();

void setup()
{
    Serial.begin(115200);

    pinMode(LED_PIN_1, OUTPUT);
    pinMode(LED_PIN_2, OUTPUT);
    pinMode(LED_PIN_3, OUTPUT);

    digitalWrite(LED_PIN_1, LOW);
    digitalWrite(LED_PIN_2, LOW);
    digitalWrite(LED_PIN_3, LOW);

    preferences.begin("wifi-creds", false);

    String ssid = preferences.getString("ssid", DEFAULT_SSID);
    String password = preferences.getString("password", DEFAULT_PASSWORD);

    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to Wi-Fi");
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000)
    {
        Serial.print(".");
        delay(300);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        wifiConnected = true;
        Serial.println();
        Serial.print("Connected with IP: ");
        Serial.println(WiFi.localIP());
        Serial.println();

        // Инициализация mDNS
        if (MDNS.begin("esp32block")) {
            Serial.println("mDNS responder started");
            Serial.println("You can now access the device at: http://esp32block.local/");
        } else {
            Serial.println("Error setting up mDNS responder!");
        }

        startWebServer();
    }
    else
    {
        // Не удалось подключиться, запускаем режим AP
        WiFi.softAP("ESP32_Config");
        IPAddress IP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(IP);
        Serial.println("Failed to connect to Wi-Fi. Starting AP mode.");
        startWebServer();
    }
}

void loop()
{
    server.handleClient();

    // Проверяем, не пора ли выключить временно включенные светодиоды
    unsigned long currentMillis = millis();

    if (led1Blinking && currentMillis - led1StartTime >= blinkInterval)
    {
        digitalWrite(LED_PIN_1, led1PermanentOn ? HIGH : LOW);
        led1Blinking = false;
    }

    if (led2Blinking && currentMillis - led2StartTime >= blinkInterval)
    {
        digitalWrite(LED_PIN_2, led2PermanentOn ? HIGH : LOW);
        led2Blinking = false;
    }

    if (led3Blinking && currentMillis - led3StartTime >= blinkInterval)
    {
        digitalWrite(LED_PIN_3, led3PermanentOn ? HIGH : LOW);
        led3Blinking = false;
    }
}

void startWebServer()
{
    // Главная страница
    server.on("/", handleRoot);

    // Страница сохранения настроек Wi-Fi
    server.on("/save", handleSave);

    // Управление LED1
    server.on("/led1_on", HTTP_GET, []() {
        led1PermanentOn = true;
        digitalWrite(LED_PIN_1, HIGH);
        server.send(200, "text/plain", "LED1 is ON permanently");
    });

    server.on("/led1_off", HTTP_GET, []() {
        led1PermanentOn = false;
        if (!led1Blinking) digitalWrite(LED_PIN_1, LOW);
        server.send(200, "text/plain", "LED1 is OFF");
    });

    server.on("/led1_blink", HTTP_GET, []() {
        led1Blinking = true;
        led1StartTime = millis();
        digitalWrite(LED_PIN_1, HIGH);
        server.send(200, "text/plain", "LED1 blinked for 1 second");
    });

    // Управление LED2
    server.on("/led2_on", HTTP_GET, []() {
        led2PermanentOn = true;
        digitalWrite(LED_PIN_2, HIGH);
        server.send(200, "text/plain", "LED2 is ON permanently");
    });

    server.on("/led2_off", HTTP_GET, []() {
        led2PermanentOn = false;
        if (!led2Blinking) digitalWrite(LED_PIN_2, LOW);
        server.send(200, "text/plain", "LED2 is OFF");
    });

    server.on("/led2_blink", HTTP_GET, []() {
        led2Blinking = true;
        led2StartTime = millis();
        digitalWrite(LED_PIN_2, HIGH);
        server.send(200, "text/plain", "LED2 blinked for 1 second");
    });

    // Управление LED3
    server.on("/led3_on", HTTP_GET, []() {
        led3PermanentOn = true;
        digitalWrite(LED_PIN_3, HIGH);
        server.send(200, "text/plain", "LED3 is ON permanently");
    });

    server.on("/led3_off", HTTP_GET, []() {
        led3PermanentOn = false;
        if (!led3Blinking) digitalWrite(LED_PIN_3, LOW);
        server.send(200, "text/plain", "LED3 is OFF");
    });

    server.on("/led3_blink", HTTP_GET, []() {
        led3Blinking = true;
        led3StartTime = millis();
        digitalWrite(LED_PIN_3, HIGH);
        server.send(200, "text/plain", "LED3 blinked for 1 second");
    });

    server.begin();
    Serial.println("HTTP server started");
}

void handleRoot()
{
    String html = "<html><body>";
    if (!wifiConnected)
    {
        html += "<h1>Wi-Fi Config</h1>";
        html += "<form action=\"/save\" method=\"post\">";
        html += "SSID: <input type=\"text\" name=\"ssid\"><br>";
        html += "Password: <input type=\"password\" name=\"password\"><br>";
        html += "<input type=\"submit\" value=\"Save\">";
        html += "</form>";
    }
    else
    {
        html += "<h1>LED Control</h1>";
        html += "<p><a href=\"/led1_on\">LED1 ON</a> | <a href=\"/led1_off\">LED1 OFF</a> | <a href=\"/led1_blink\">LED1 BLINK</a></p>";
        html += "<p><a href=\"/led2_on\">LED2 ON</a> | <a href=\"/led2_off\">LED2 OFF</a> | <a href=\"/led2_blink\">LED2 BLINK</a></p>";
        html += "<p><a href=\"/led3_on\">LED3 ON</a> | <a href=\"/led3_off\">LED3 OFF</a> | <a href=\"/led3_blink\">LED3 BLINK</a></p>";
    }
    html += "</body></html>";

    server.send(200, "text/html", html);
}

void handleSave()
{
    if (server.method() == HTTP_POST)
    {
        String ssid = server.arg("ssid");
        String password = server.arg("password");

        preferences.putString("ssid", ssid);
        preferences.putString("password", password);

        server.send(200, "text/html", "<html><body><h1>Saved. Rebooting...</h1></body></html>");

        delay(1000);
        ESP.restart();
    }
    else
    {
        server.send(405, "text/plain", "Method Not Allowed");
    }
}
