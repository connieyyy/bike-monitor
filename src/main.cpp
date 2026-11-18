#include <Arduino.h>
#include "bike_motion.h"
#include "bike_button.h"
#include "bike_display.h"
#include "bike_alarm.h"
#include "bike_cloud.h"

bool bikeMoving = false;
float motionValueG = 0.0f;
bool armed = true;
bool alarmTriggered = false;

void setup() {
    bikeDisplayBegin();
    bikeAlarmBegin();
    bikeMotionBegin();
    bikeButtonBegin();
    bikeCloudBegin();
}

void loop() {
    bikeMotionUpdate(bikeMoving, motionValueG);
    armed = bikeButtonUpdate(bikeMoving, alarmTriggered);
    bool alarmTriggered = bikeAlarmUpdate(armed, bikeMoving);
    bool cloudOnline = bikeCloudUpdate(armed, bikeMoving, alarmTriggered, motionValueG);

    bikeDisplayUpdate(armed, bikeMoving, alarmTriggered, motionValueG, cloudOnline);
}
