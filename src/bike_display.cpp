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

void bikeDisplayUpdate(bool armed, bool bikeMoving, bool alarmTriggered, float motionValueG, bool cloudOnline) {
    static bool previousStatus = false;
    static bool firstUpdate = true;
    static uint32_t lastOledUpdate = 0;

    if (firstUpdate || bikeMoving != previousStatus) {
        Serial.print("BIKE: ");
        Serial.println(bikeMoving ? "MOVING" : "STATIONARY");

        previousStatus = bikeMoving;
        firstUpdate = false;
    }

    if (iotEvery(200, lastOledUpdate)) {
        String stateLine = alarmTriggered ? "ALARM TRIGGERED!" : (armed ? "System: ARMED" : "System: DISARMED");
        String motionLine = (bikeMoving ? "MOVING " : "STOPPED ") + String(motionValueG, 2) + "g";
        String cloudLine = cloudOnline ? "Cloud: ONLINE" : "Cloud: OFFLINE";

        iotDisplayLines("Bike Monitor", stateLine, motionLine, cloudLine);
    }
}
