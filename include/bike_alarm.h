#ifndef BIKE_ALARM_H
#define BIKE_ALARM_H

enum class BikeAlarmState {
    Clear,
    Warning,
    Triggered
};

void bikeAlarmBegin();
BikeAlarmState bikeAlarmUpdate(bool armed, bool bikeMoving, float motionValueG);

#endif
