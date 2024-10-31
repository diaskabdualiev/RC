#include <Arduino.h>
#include <Update.h>
#include <WiFi.h>
#include <FirebaseClient.h>
#include <Preferences.h>
#include <WebServer.h>

#define WIFI_SSID "RCAlish"
#define WIFI_PASSWORD "alishalish"

// The API key can be obtained from Firebase console > Project Overview > Project settings.
#define API_KEY "AIzaSyDyAdY11A2KSgf_ggE4Z3GiFOyzzFAek5k"

// User Email and password that already registered or added in your project.
#define USER_EMAIL "alish@alish.com"
#define USER_PASSWORD "alishalish"
#define DATABASE_URL "https://rcalish-44653-default-rtdb.asia-southeast1.firebasedatabase.app"

void asyncCB(AsyncResult &aResult);
void printResult(AsyncResult &aResult);
void startWebServer();
void handleRoot();
void handleSave();

DefaultNetwork network; // Initialize with boolean parameter to enable/disable network reconnection
UserAuth user_auth(API_KEY, USER_EMAIL, USER_PASSWORD);
FirebaseApp app;

#include <WiFiClientSecure.h>
WiFiClientSecure ssl_client1, ssl_client2;

using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client1, getNetwork(network)), aClient2(ssl_client2, getNetwork(network));
unsigned long previousMillis = 0;
unsigned long count = 0;

RealtimeDatabase Database;

#define LED_PIN_1 25
#define LED_PIN_2 32
#define LED_PIN_3 33

unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
unsigned long previousMillis3 = 0;

const long interval = 1000;  // Интервал в 1 секунду

bool button1Pressed = false; // Флаги для кнопок 1-3
bool button2Pressed = false;
bool button3Pressed = false;

unsigned long button1StartTime = 0; // Время начала для кнопок 1-3
unsigned long button2StartTime = 0;
unsigned long button3StartTime = 0;

// Флаги для постоянного включения светодиодов (кнопки 4-6)
bool led1PermanentOn = false;
bool led2PermanentOn = false;
bool led3PermanentOn = false;

Preferences preferences;
WebServer server(80);
bool wifiConnected = false;

void setup()
{
    pinMode(LED_PIN_1, OUTPUT);
    pinMode(LED_PIN_2, OUTPUT);
    pinMode(LED_PIN_3, OUTPUT);

    // По умолчанию светодиоды выключены
    digitalWrite(LED_PIN_1, LOW);
    digitalWrite(LED_PIN_2, LOW);
    digitalWrite(LED_PIN_3, LOW);

    Serial.begin(115200);

    // Инициализируем Preferences
    preferences.begin("wifi-creds", false);

    String ssid = preferences.getString("ssid", WIFI_SSID);
    String password = preferences.getString("password", WIFI_PASSWORD);

    wifiConnected = false;

    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.print("Connecting to Wi-Fi");
    unsigned long startAttemptTime = millis();

    // Попытка подключения в течение 10 секунд
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
    }

    if (wifiConnected)
    {
        // Продолжаем с инициализацией Firebase
        Firebase.printf("Firebase Client v%s\n", FIREBASE_CLIENT_VERSION);
        Serial.println("Initializing app...");

        ssl_client1.setInsecure();
        ssl_client2.setInsecure();

        initializeApp(aClient2, app, getAuth(user_auth), asyncCB, "authTask");

        // Binding the FirebaseApp for authentication handler.
        // To unbind, use Database.resetApp();
        app.getApp<RealtimeDatabase>(Database);

        Database.url(DATABASE_URL);

        // Устанавливаем фильтры для SSE (если необходимо)
        Database.setSSEFilters("get,put,patch,keep-alive,cancel,auth_revoked");

        // Подписываемся на изменения в базе данных
        Database.get(aClient, "/alish", asyncCB, true /* SSE mode (HTTP Streaming) */, "streamTask");
    }
    else
    {
        // Запускаем точку доступа и веб-сервер для ввода новых Wi-Fi настроек
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
    if (!wifiConnected)
    {
        server.handleClient();
    }
    else
    {
        // Асинхронные задачи должны выполняться в основном цикле
        app.loop();
        Database.loop();

        unsigned long currentMillis = millis();

        // Пример отправки данных в базу каждые 2 секунды
        if (currentMillis - previousMillis >= 2000 && app.ready())
        {
            previousMillis = currentMillis;
            Database.set<int>(aClient2, "/alish1/count1", count++, asyncCB, "setCountTask");
        }

        // Обработка светодиодов 1-3 (кнопки 1-3)
        if (button1Pressed)
        {
            if (currentMillis - button1StartTime >= interval)
            {
                // Возвращаем светодиод в соответствующее состояние
                if (led1PermanentOn)
                    digitalWrite(LED_PIN_1, HIGH); // Включаем светодиод постоянно
                else
                    digitalWrite(LED_PIN_1, LOW);  // Выключаем светодиод

                button1Pressed = false;         // Сбрасываем флаг
                Serial.println("Светодиод 1 вернулся в исходное состояние.");
            }
        }

        if (button2Pressed)
        {
            if (currentMillis - button2StartTime >= interval)
            {
                if (led2PermanentOn)
                    digitalWrite(LED_PIN_2, HIGH);
                else
                    digitalWrite(LED_PIN_2, LOW);

                button2Pressed = false;
                Serial.println("Светодиод 2 вернулся в исходное состояние.");
            }
        }

        if (button3Pressed)
        {
            if (currentMillis - button3StartTime >= interval)
            {
                if (led3PermanentOn)
                    digitalWrite(LED_PIN_3, HIGH);
                else
                    digitalWrite(LED_PIN_3, LOW);

                button3Pressed = false;
                Serial.println("Светодиод 3 вернулся в исходное состояние.");
            }
        }
    }
}

void startWebServer()
{
    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.begin();
    Serial.println("HTTP server started");
}

void handleRoot()
{
    String html = "<html><body>";
    html += "<h1>Wi-Fi Config</h1>";
    html += "<form action=\"/save\" method=\"post\">";
    html += "SSID: <input type=\"text\" name=\"ssid\"><br>";
    html += "Password: <input type=\"password\" name=\"password\"><br>";
    html += "<input type=\"submit\" value=\"Save\">";
    html += "</form></body></html>";

    server.send(200, "text/html", html);
}

void handleSave()
{
    if (server.method() == HTTP_POST)
    {
        String ssid = server.arg("ssid");
        String password = server.arg("password");

        // Сохраняем в Preferences
        preferences.putString("ssid", ssid);
        preferences.putString("password", password);

        // Отправляем ответ
        server.send(200, "text/html", "<html><body><h1>Saved. Rebooting...</h1></body></html>");

        delay(1000);

        // Перезагружаем устройство
        ESP.restart();
    }
    else
    {
        server.send(405, "text/plain", "Method Not Allowed");
    }
}

void asyncCB(AsyncResult &aResult)
{
    // Не размещайте тяжелый код здесь
    printResult(aResult);
}

void printResult(AsyncResult &aResult)
{
    if (aResult.isEvent())
    {
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.appEvent().message().c_str(), aResult.appEvent().code());
    }

    if (aResult.isDebug())
    {
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());
    }

    if (aResult.isError())
    {
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());
    }

    if (aResult.available())
    {
        RealtimeDatabaseResult &RTDB = aResult.to<RealtimeDatabaseResult>();
        if (RTDB.isStream())
        {
            Serial.println("----------------------------");
            Firebase.printf("task: %s\n", aResult.uid().c_str());
            Firebase.printf("event: %s\n", RTDB.event().c_str());
            Firebase.printf("path: %s\n", RTDB.dataPath().c_str());
            Firebase.printf("data: %s\n", RTDB.to<const char *>());
            Firebase.printf("type: %d\n", RTDB.type());

            // Получаем путь данных (например, "/button1", "/button2", ...)
            String payload = RTDB.dataPath();
            payload.trim();

            // Устанавливаем флаги и время начала на основе полученного пути
            if (payload == "/button1")
            {
                button1Pressed = true;
                button1StartTime = millis();  // Сохраняем текущее время
                digitalWrite(LED_PIN_1, HIGH); // Временно включаем светодиод
                Serial.println("Светодиод 1 временно включен на 1 секунду.");
            }
            else if (payload == "/button2")
            {
                button2Pressed = true;
                button2StartTime = millis();
                digitalWrite(LED_PIN_2, HIGH);
                Serial.println("Светодиод 2 временно включен на 1 секунду.");
            }
            else if (payload == "/button3")
            {
                button3Pressed = true;
                button3StartTime = millis();
                digitalWrite(LED_PIN_3, HIGH);
                Serial.println("Светодиод 3 временно включен на 1 секунду.");
            }
            else if (payload == "/button4")
            {
                // Изменяем состояние постоянного включения для светодиода 1
                led1PermanentOn = !led1PermanentOn;
                if (led1PermanentOn)
                {
                    digitalWrite(LED_PIN_1, HIGH); // Включаем светодиод постоянно
                    Serial.println("Светодиод 1 постоянно включен.");
                }
                else
                {
                    digitalWrite(LED_PIN_1, LOW); // Выключаем светодиод
                    Serial.println("Светодиод 1 выключен.");
                }
            }
            else if (payload == "/button5")
            {
                led2PermanentOn = !led2PermanentOn;
                if (led2PermanentOn)
                {
                    digitalWrite(LED_PIN_2, HIGH);
                    Serial.println("Светодиод 2 постоянно включен.");
                }
                else
                {
                    digitalWrite(LED_PIN_2, LOW);
                    Serial.println("Светодиод 2 выключен.");
                }
            }
            else if (payload == "/button6")
            {
                led3PermanentOn = !led3PermanentOn;
                if (led3PermanentOn)
                {
                    digitalWrite(LED_PIN_3, HIGH);
                    Serial.println("Светодиод 3 постоянно включен.");
                }
                else
                {
                    digitalWrite(LED_PIN_3, LOW);
                    Serial.println("Светодиод 3 выключен.");
                }
            }
        }
        else
        {
            Serial.println("----------------------------");
            Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
        }

#if defined(ESP32) || defined(ESP8266)
        Firebase.printf("Free Heap: %d\n", ESP.getFreeHeap());
#elif defined(ARDUINO_RASPBERRY_PI_PICO_W)
        Firebase.printf("Free Heap: %d\n", rp2040.getFreeHeap());
#endif
    }
}
