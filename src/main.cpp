#include <Arduino.h>
#include "bike_motion.h"
#include "bike_button.h"
#include "bike_display.h"
#include "bike_alarm.h"
#include "bike_cloud.h"

bool bikeMoving = false;
float motionValueG = 0.0f;
bool armed = true;
BikeAlarmState alarmState = BikeAlarmState::Clear;

void setup() {
    bikeDisplayBegin();
    bikeAlarmBegin();
    bikeMotionBegin();
    bikeButtonBegin();
    bikeCloudBegin();
}

void loop() {
    bikeMotionUpdate(bikeMoving, motionValueG);
    armed = bikeButtonUpdate(bikeMoving, alarmState == BikeAlarmState::Triggered);
    alarmState = bikeAlarmUpdate(armed, bikeMoving, motionValueG);
    bool cloudOnline = bikeCloudUpdate(armed, bikeMoving, alarmState, motionValueG);

    bikeDisplayUpdate(armed, bikeMoving, alarmState, motionValueG, cloudOnline);
}
