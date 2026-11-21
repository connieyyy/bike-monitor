#ifndef BIKE_CLOUD_H
#define BIKE_CLOUD_H

#include "bike_alarm.h"
#include "bike_button.h"

void bikeCloudBegin();
bool bikeCloudUpdate(bool armed, bool bikeMoving, BikeAlarmState alarmState, float motionValueG,
                     BikeDisarmAttempt disarmAttempt);

#endif
