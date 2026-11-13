#ifndef BIKE_DISPLAY_H
#define BIKE_DISPLAY_H

void bikeDisplayBegin();
void bikeDisplayUpdate(bool armed, bool bikeMoving, bool alarmTriggered, float motionValueG, bool cloudOnline);
#endif