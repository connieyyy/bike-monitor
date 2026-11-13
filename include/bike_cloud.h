#ifndef BIKE_CLOUD_H
#define BIKE_CLOUD_H

void bikeCloudBegin();
bool bikeCloudUpdate(bool armed, bool bikeMoving, bool alarmTriggered, float motionValueG);

#endif