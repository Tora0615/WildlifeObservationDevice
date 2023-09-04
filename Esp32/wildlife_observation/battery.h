// #include "driver/adc.h"   -->  for cpp dev only; 
// For arduino, don't need to include anything.
// The ref code is in esp32-hal-adc.c
#ifndef BATTERY_H
#define BATTERY_H


#define BATTERY_MONITOR_PIN A0
#define ADC_BIT_12 12
#define FULL_BATTERY_VOLTAGE 4.2    //battery : 4.2
#define EMPTY_BATTERY_VOLTAGE 2.55  // protect board : 2.54 +- 0.1 / battery : 2.5 
const float voltage_division_factor = 0.2272;  // 150k : 510k -->  150 / (150 + 510) = 0.2272

#include "sd_operation.h"
#include <SimpleKalmanFilter.h>
SimpleKalmanFilter simpleKalmanFilter(2, 2, 0.01);


void batteryMonitorInit(){
  pinMode(BATTERY_MONITOR_PIN, INPUT);        // GPIO36 (A0) (read only)
  analogSetAttenuation(ADC_2_5db);            // max 1.5V
  analogSetWidth(ADC_BIT_12);                 // 12 bit -> 0 ~ 4095

  /*
  ADC_0db,    1.1V
  ADC_2_5db,  1.5V
  ADC_6db,    2.2V
  ADC_11db    3.9V（實際最大採集到3.3V電壓）

  https://kknews.cc/digital/lgmbkjz.html
  */

  // put data into the filter
  for (int i =0; i<10; i++){
    simpleKalmanFilter.updateEstimate(analogReadMilliVolts(BATTERY_MONITOR_PIN));
  }

  // Write log
  writeMsgToPath(systemLogPath, "Battery monitor init successful");
}

float getBatteryVoltage(){
  float estimated_value = simpleKalmanFilter.updateEstimate(analogReadMilliVolts(BATTERY_MONITOR_PIN));
  return estimated_value / 1000.0 / voltage_division_factor;
}

float getBatteryPercentage(){
  
  const float calculated_full_batter_voltage = FULL_BATTERY_VOLTAGE * voltage_division_factor;
  const float calculated_empty_batter_voltage = EMPTY_BATTERY_VOLTAGE * voltage_division_factor;

  float estimated_value = simpleKalmanFilter.updateEstimate(analogReadMilliVolts(BATTERY_MONITOR_PIN));
  float percentage = (estimated_value / 1000.0 - calculated_empty_batter_voltage) / (calculated_full_batter_voltage - calculated_empty_batter_voltage);
  percentage = percentage < 0 ? 0 : percentage;
  return percentage * 100;  
}
              


#endif