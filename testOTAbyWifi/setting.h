#ifndef SETTING_H
#define SETTING_H

// This status will only keep when light/deep sleep (reset will clean the status)
RTC_DATA_ATTR bool isFirstBoot = true;
bool isNeedToUpdate = false;

#endif