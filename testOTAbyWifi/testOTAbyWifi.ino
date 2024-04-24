#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
// #include <ESPmDNS.h>
#include <Update.h>

// const char* host = "esp32";
const char* ssid = "IMTT_5F_Guest";
const char* password = "035280111";

WebServer server(80);

const char* serverIndex = 
/*--- css  ---*/
"<style>"
  "#progress_bar_container { display: none; }" // 初始隱藏進度條
"</style>"

/*--- html  ---*/
"<div id='upload_form_container'>"
  "<form id='upload_form' enctype='multipart/form-data' method='post'>"
    "<input type='file' name='update' id='file'>"
    "<input type='submit' value='Update'>"
  "</form>"
"</div>"
"<div id='progress_bar_container'>" // 包裹進度條的容器
  "<progress id='progress_bar' max='100' value='0'></progress>"
  "<div id='progress'></div>"
  "<div id='msg'></div>"
"</div>"

/*--- js ---*/
"<script>"
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
          "document.getElementById('progress').textContent = 'Upload finished! Device will auto restart.';"
          "document.getElementById('msg').textContent = 'You can close the window now.';"
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
"</script>";


/*
 * setup function
 */
void setup(void) {
  Serial.begin(115200);

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}

void loop(void) {
  server.handleClient();
  delay(1);
}
