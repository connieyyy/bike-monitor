#include <Arduino.h>
#include "iot_common.h"
#include "iot_display.h"
#include "bike_display.h"

// This module ONLY initializes and updates the OLED and Serial Monitor.
// It does not read sensors or control buttons, LEDs, or a buzzer.

void bikeDisplayBegin() {
    iotStartSerial("Bike Monitor Display");
    iotInitI2C();
    iotInitSPI();
    iotInitDisplay();

    Serial.println("Bike monitor display ready.");
}

void bikeDisplayUpdate(bool bikeMoving, float motionValueG) {
    static bool previousStatus = false;
    static bool firstUpdate = true;
    static uint32_t lastOledUpdate = 0;
    static uint32_t lastSerialUpdate = 0;

    if (firstUpdate || bikeMoving != previousStatus) {
        Serial.print("BIKE: ");
        Serial.println(bikeMoving ? "MOVING" : "STATIONARY");

        previousStatus = bikeMoving;
        firstUpdate = false;
    }

    if (iotEvery(500, lastSerialUpdate)) {
        Serial.print("Motion value: ");
        Serial.print(motionValueG, 3);
        Serial.println(" g");
    }

    if (iotEvery(200, lastOledUpdate)) {
        String statusLine = bikeMoving ? "Status: MOVING" : "Status: STOPPED";
        String valueLine = "Motion: " + String(motionValueG, 2) + " g";

        iotDisplayLines("Bike Monitor", "BIKE STATUS", statusLine, valueLine);
    }
}
