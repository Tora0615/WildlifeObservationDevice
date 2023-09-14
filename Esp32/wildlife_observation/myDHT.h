#ifndef MYDHT_H
#define MYDHT_H

#include <DHT.h>
#define DHTPIN 33
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

void DHT_init(){
  dht.begin();
}


float DHT_get_temperature(){
  const float sample_time = 10.0;
  float sum = 0;
  for (int i = 0; i<sample_time; i++){
    sum += dht.readTemperature();
    delay(50);
  }
  return sum/sample_time;
}


float DHT_get_Humidity(){
  const float sample_time = 10.0;
  float sum = 0;
  for (int i = 0; i<sample_time; i++){
    sum += dht.readHumidity();
    delay(50);
  }
  return sum/sample_time;
}


#endif

// #define DHT_DS18B20_POWERPIN 32