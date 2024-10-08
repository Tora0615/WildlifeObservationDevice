#include <Wire.h>
#include <RTClib.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>

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

"<body>"
  "<h2>RTC Time</h2>"
  "<p>Current Time: <span id='current-time'>Loading...</span></p>"
  "<p>"
    "<label for='datetime'>Set Time (YYYY-MM-DD HH:MM:SS):</label>"
    "<input type='text' id='datetime'>"
    "<button onclick='setTime()'>Set Time</button>"
  "</p>"
  "<p id='set-time-result'></p>"

  "<h2>Firmware upload</h2>"
  "<div id='upload_form_container'>"
    "<form id='upload_form' enctype='multipart/form-data' method='post'>"
      "<input type='file' name='update' id='file'>"
      "<input type='submit' value='Update'>"
    "</form>"
  "</div>"
  "<div id='progress_bar_container'>"
    "<progress id='progress_bar' max='100' value='0'></progress>"
    "<div id='progress'></div>"
    "<div id='msg'></div>"
  "</div>"
"</body>"

"<head>"
  "<title>Setting page</title>"
  "<style>"
    "#progress_bar_container { display: none; }"
  "</style>"
  "<script>"
    "async function updateTime() {"
      "const response = await fetch('/time');"
      "const time = await response.text();"
      "document.getElementById('current-time').innerText = time;"
    "};"

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
      "setTimeout(clearSetTimeResult, 3000);"
    "};"

    "function clearSetTimeResult() {"
      "document.getElementById('set-time-result').innerText = '';"
    "}"

    "window.onload = function() {"
      "updateTime();"
      "setInterval(updateTime, 1000);"
    "};"

    // 監聽表單提交事件
    "document.getElementById('upload_form').addEventListener('submit', function(event) {"
      // 取得文件輸入元素
      "var fileInput = document.getElementById('file');"
      // 取得第一個選取的文件
      "var file = fileInput.files[0];"
      // 建立 FormData 物件，將文件加入並設定名稱為 'update'
      "var formData = new FormData();"
      "formData.append('update', file, file.name);"
      // 建立 XMLHttpRequest 物件
      "var xhr = new XMLHttpRequest();"
      // 開啟一個 POST 請求，目標 URL 為 '/update'
      "xhr.open('POST', '/update', true);"
      // 監聽上傳進度事件
      "xhr.upload.addEventListener('progress', function(event) {"
        // 若能計算上傳進度
        "if (event.lengthComputable) {"
          // 計算並顯示上傳進度百分比
          "var percentComplete = event.loaded / event.total;"
          // text
          "document.getElementById('progress').textContent = Math.round(percentComplete * 100) + '%';"
          // progress bar
          "document.getElementById('progress_bar').value = Math.round(percentComplete * 100);"
          "if (percentComplete == 1) {"
            "document.getElementById('progress').textContent = 'Upload finished! Device will auto restart. You can close the window now.';"
            "document.getElementById('msg').textContent = 'Dont forget to press reset button after you disconnect from wifi named ESP32!';"
          "}"
        "}"
      "});"
      // 監聽上傳完成事件
      "xhr.addEventListener('load', function(event) {"
        // 當上傳完成時輸出訊息
        "console.log('Upload complete');"
      "});"
      // 發送 HTTP 請求，傳送 FormData 物件
      "xhr.send(formData);"
      // 顯示進度條容器
      "document.getElementById('progress_bar_container').style.display = 'block';"
      // 阻止預設的表單提交行為
      "event.preventDefault();"
    "}, false);"  // 冒泡階段（Bubble Phase）(default)
    
  "</script>"
"</head>"

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

  // 處理固件更新
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
    bool updateHasError = Update.hasError();
    request->send(200, "text/plain", (updateHasError) ? "FAIL" : "OK");
    if (!updateHasError) {
      ESP.restart();
    }
  }, 
  [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    if (!index) {
      Serial.printf("Update: %s\n", filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // Start with max available size
        Update.printError(Serial);
      }
    }
    if (Update.write(data, len) != len) {
      Update.printError(Serial);
    }
    if (final) {
      if (Update.end(true)) { // True to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", index + len);
      } else {
        Update.printError(Serial);
      }
    }
  });

  // 啟動 Web Server
  server.begin();

  // test will web server block the code or not 
  Serial.println("I'm here"); // the real situation is : "YES"
}

void loop() {
  // 空的loop函數，因為Web Server處理一切
}
