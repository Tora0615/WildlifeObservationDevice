uint32_t sys_RTC_time_offset;         // RTC clock. It will only change when first boot or date changed. First day equal the boot time, others day it will allways close to 0
uint32_t sys_millis_time_offset;    // millis clock.

uint32_t getPassedSecOfToday(){
  return sys_RTC_time_offset + (millis() - sys_millis_time_offset)/1000;  // RTC time when boot + (time counter now - time counter before)
}

void checkDayChange(){
  if( getPassedSecOfToday() > 86400){  // a day change
    sys_RTC_time_offset = GetHowManySecondsHasPassedTodayFromRtc();
    sys_millis_time_offset = millis();
  }
}

