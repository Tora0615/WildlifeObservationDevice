#include "setting.h"

AsyncWebServer server(80);

// save HTML page as variable
const char* serverIndex = 
"<!DOCTYPE html>"
"<html>"
  "<body>"
    /*--- the area of rtc ---*/
    #ifdef HTML_ZH
      "<h2>RTC 時間</h2>"
      //// realtime rtc refresh
      "<p>現在時間 : <span id='current-time'> 載入中...</span></p>"
      //// set time area
      "<p>"
        "<label for='datetime'>設置 (YYYY-MM-DD HH:MM:SS) --- </label>"
        "<input type='text' id='datetime'>"
        "<button onclick='setTime()'>更新</button>"
      "</p>"
    #else
      "<h2>RTC Time</h2>"
      //// realtime rtc refresh
      "<p>Current Time : <span id='current-time'>Loading...</span></p>"
      //// set time area
      "<p>"
        "<label for='datetime'>Set Time (YYYY-MM-DD HH:MM:SS) --- </label>"
        "<input type='text' id='datetime'>"
        "<button onclick='setTime()'>update</button>"
      "</p>"
    #endif
    //// update result, only shown 3 sec
    "<p id='set-time-result'></p>"

    /*--- the area of firmware upload ---*/
    #ifdef HTML_ZH
      "<h2>韌體更新</h2>"
    #else
      "<h2>Firmware upload</h2>"
    #endif
    //// file select area
    "<div id='upload_form_container'>"
      "<form id='upload_form' enctype='multipart/form-data' method='post'>"
        "<input type='file' name='update' id='file'>"
        "<input type='submit' value='Update'>"
      "</form>"
    "</div>"
    //// upload status
    "<div id='progress_bar_container'>"
      "<progress id='progress_bar' max='100' value='0'></progress>"
      "<div id='progress'></div>"
      "<div id='msg'></div>"
    "</div>"
  "</body>"

  "<head>"
    /*--- encoding type ---*/
    "<meta charset='UTF-8'>"

    /*--- browser tab name ---*/
    "<title>Setting page</title>"

    /*--- css ---*/
    "<style>"
      "#progress_bar_container { display: none; }"
    "</style>"

    /*--- js ---*/
    "<script>"
      /*--- how the web get the current time by api ---*/
      "async function updateTime() {"
        "const response = await fetch('/time');"
        "const time = await response.text();"
        "document.getElementById('current-time').innerText = time;"
      "};"

      /*--- the thing need to do when press setTime ---*/
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

      /*--- 3 sec after rtc update, clear showing status automatically ---*/
      "function clearSetTimeResult() {"
        "document.getElementById('set-time-result').innerText = '';"
      "};"

      /*--- auto refresh timer ---*/
      "window.onload = function() {"
        "updateTime();"
        "setInterval(updateTime, 1000);"
      "};"

      // 監聽表單提交事件
      "document.getElementById('upload_form').addEventListener('submit', function(event) {"
        /*--- 檢查部分 ---*/
        // 檢查是否選擇了文件 
        "var fileInput = document.getElementById('file');"
        "if (fileInput.files.length === 0) {"
          // 彈出式警告
          "alert('Please select a file.');"
          "event.preventDefault();"
          "return;"
        "}"

        // 檢查文件是否為.ino.bin結尾 
        "var file = fileInput.files[0];"
        "if (!file.name.endsWith('.ino.bin')) {"
          // 彈出式警告
          "alert('Invalid file type. Please select a .ino.bin file.');"
          "event.preventDefault();"
          "return;"
        "}"

        /*--- 正式處理邏輯 ---*/
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
              #ifdef HTML_ZH
                "document.getElementById('progress').textContent = '完成!';"
                "document.getElementById('msg').textContent = '請記得切斷ESP32連線並重啟!';"
              #else
                "document.getElementById('progress').textContent = 'Upload finished!';"
                "document.getElementById('msg').textContent = 'Dont forget to reset ESP32 after you disconnected from its wifi!';"
              #endif
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


void startUpdateServer(){
  Serial.println("  |-- webserver");

  // root page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", serverIndex);
  });

  // current time api
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request){
    DateTime now = rtc.now();
    char buffer[30];
    snprintf(buffer, 30, "%02d-%02d-%02d %02d:%02d:%02d", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
    request->send(200, "text/plain", buffer);
  });

  // set time api
  server.on("/set_time", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("datetime", true)) {
      String datetime = request->getParam("datetime", true)->value();
      int year, month, day, hour, minute, second;
      // 檢查輸入格式是否正確
      int parsed = sscanf(datetime.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
      if (parsed == 6 && year >= 2000 && year <= 2099 && month >= 1 && month <= 12 &&
          day >= 1 && day <= 31 && hour >= 0 && hour <= 23 && minute >= 0 && minute <= 59 &&
          second >= 0 && second <= 59) {
        rtc.adjust(DateTime(year, month, day, hour, minute, second));
        #ifdef HTML_ZH
          request->send(200, "text/plain", "時間更新完成!");
        #else
          request->send(200, "text/plain", "Time updated!");
        #endif
      } else {
        #ifdef HTML_ZH
          request->send(400, "text/plain", "錯誤,請檢察輸入");
        #else
          request->send(400, "text/plain", "error, check your input");
        #endif
      }
    } else {
      request->send(400, "text/plain", "Bad Request");
    }
  });

  // firmware update api
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

  // start web server
  server.begin();
}