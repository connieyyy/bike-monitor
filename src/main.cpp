#include <Arduino.h>
#include "bike_display.h"


bool bikeMoving = false;
float motionValueG = 0.0;

void setup() {
    bikeDisplayBegin();
}

void loop() {
    // Update OLED and Serial Monitor.
    bikeDisplayUpdate(bikeMoving, motionValueG);
}
