#include "setting.h"
#define AP_TIMEOUT_MS 30000  // 30 sec

WebServer server(80);
const char* ssid = "ESP32";

void scanConnectedForMS(int countDownTime){
  unsigned long startTime = millis();
  unsigned long printTimer = startTime;
  int printTimerInterval = 5000; // ms
  while(1){
    // check is over time or not
    if ( (millis() - startTime) > countDownTime){
      Serial.println("Not client connected, closing the AP...");
      break;
    }

    // do one wifi scan
    int numDevices = WiFi.softAPgetStationNum();

    // print status
    if ((millis() - printTimer) >= printTimerInterval){
      Serial.print("Number of devices found: ");
      Serial.println(numDevices);
      Serial.println("remain sec of AP : " + String( (countDownTime - (millis() - startTime))/1000.f ) );
      printTimer = millis();
    }
    // check the num of connected
    if (numDevices != 0){
      // escape the while loop
      Serial.print("Setup update service...");
      isNeedToUpdate = true;
      break;
    }
    // delay a little bit for next scan
    delay (10);
  } 
}



/* Init soft ap service and show basic info*/
void startSoftAp(){
  // create a soft ip
  if (!WiFi.softAP(ssid)) {
    log_e("Soft AP creation failed.");
    while(1);
  }
  // Set static IP after created
  IPAddress AP_LOCAL_IP(192, 168, 1, 168);
  IPAddress AP_GATEWAY_IP(192, 168, 1, 4);
  IPAddress AP_NETWORK_MASK(255, 255, 255, 0);
  if (!WiFi.softAPConfig(AP_LOCAL_IP, AP_GATEWAY_IP, AP_NETWORK_MASK)) {
    Serial.println("AP Config Failed");
    return;
  }
  // show current 
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  // open ap for a short time
  scanConnectedForMS(AP_TIMEOUT_MS);
}

/* we don't need to turn off after firmware updated */
/* system will automatically restart */
void closeSoftAP(){
  // turn off SoftAP
  WiFi.softAPdisconnect(true);
}
