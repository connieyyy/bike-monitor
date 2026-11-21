#ifndef BIKE_DISPLAY_H
#define BIKE_DISPLAY_H

#include "bike_alarm.h"

void bikeDisplayBegin();
void bikeDisplayUpdate(bool armed, bool bikeMoving, BikeAlarmState alarmState, float motionValueG, bool cloudOnline);
#endif
