#include <Wire.h>
#include <RTClib.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// DS3231 RTC模塊實例
RTC_DS3231 rtc;

// WiFi配置
const char* ssid = "IMTT_5F";
const char* password = "035628111";

// Web Server實例
AsyncWebServer server(80);

// HTML頁面作為變數存儲
const char* serverIndex = 
"<!DOCTYPE html>"
"<html>"
"<head>"
  "<title>RTC Time</title>"
  "<script>"
    "async function updateTime() {"
      "const response = await fetch('/time');"
      "const time = await response.text();"
      "document.getElementById('current-time').innerText = time;"
    "}"

    "async function setTime() {"
      "const datetime = document.getElementById('datetime').value;"
      "const response = await fetch('/set_time', {"
        "method: 'POST',"
        "headers: {"
          "'Content-Type': 'application/x-www-form-urlencoded'"
        "},"
        "body: `datetime=${encodeURIComponent(datetime)}`"
      "});"
      "const result = await response.text();"
      "document.getElementById('set-time-result').innerText = result;"
      "updateTime();"
    "}"

    "window.onload = function() {"
      "updateTime();"
      "setInterval(updateTime, 1000);"
    "}"
  "</script>"
"</head>"
"<body>"
  "<h1>RTC Time</h1>"
  "<p>Current Time: <span id='current-time'>Loading...</span></p>"
  "<p>"
    "<label for='datetime'>Set Time (YYYY-MM-DD HH:MM:SS):</label>"
    "<input type='text' id='datetime'>"
    "<button onclick='setTime()'>Set Time</button>"
  "</p>"
  "<p id='set-time-result'></p>"
"</body>"
"</html>";

void setup() {
  // 啟動串口
  Serial.begin(115200);

  // 初始化RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // 連接WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // 設置根目錄的網頁處理函數
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", serverIndex);
  });

  // 提供當前時間的API
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request){
    DateTime now = rtc.now();
    char buffer[30];
    snprintf(buffer, 30, "%02d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    request->send(200, "text/plain", buffer);
  });

  // 設置時間的API
  server.on("/set_time", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("datetime", true)) {
      String datetime = request->getParam("datetime", true)->value();
      int year, month, day, hour, minute, second;
      sscanf(datetime.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
      rtc.adjust(DateTime(year, month, day, hour, minute, second));
      request->send(200, "text/plain", "Time updated");
    } else {
      request->send(400, "text/plain", "Bad Request");
    }
  });

  // 啟動Web Server
  server.begin();
}

void loop() {
  // 空的loop函數，因為Web Server處理一切
}
