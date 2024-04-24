WebServer server(80);
const char* ssid = "ESP32";

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
  // show status
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}
