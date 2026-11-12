#include <Arduino.h>
#include "bike_motion.h"
#include "bike_button.h"
#include "bike_display.h"
#include "bike_alarm.h"

bool bikeMoving = false;
float motionValueG = 0.0;

void setup() {
    bikeDisplayBegin();
    bikeAlarmBegin();
    bikeMotionBegin();
    bikeButtonBegin();
}

void loop() {
    bikeMotionUpdate(bikeMoving, motionValueG);
    bool armed = bikeButtonUpdate();

    bikeDisplayUpdate(bikeMoving, motionValueG);
    bikeAlarmUpdate(armed, bikeMoving);
}
