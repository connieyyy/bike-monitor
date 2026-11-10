#include <Arduino.h>
#include "bike_display.h"
#include "bike_alarm.h"


bool bikeMoving = false;
float motionValueG = 0.0;

void setup() {
    bikeDisplayBegin();
    bikeAlarmBegin();
}

void loop() {
    // Update OLED and Serial Monitor.
    bikeDisplayUpdate(bikeMoving, motionValueG);
    bikeAlarmUpdate(bikeMoving);
}
