#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <WiFiAP.h>
#include "softAp.h"
#include "updateServer.h"


void setup(void) {
  Serial.begin(115200);
  
  startSoftAp();
  startUpdateServer();
}

void loop(void) {

}
