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

void bikeDisplayUpdate(bool armed, bool bikeMoving, BikeAlarmState alarmState, float motionValueG, bool cloudOnline) {
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
        const char* stateLine = armed ? "System: ARMED" : "System: DISARMED";
        switch (alarmState) {
            case BikeAlarmState::Clear:
                break;
            case BikeAlarmState::Warning:
                stateLine = "Motion Warning!";
                break;
            case BikeAlarmState::Triggered:
                stateLine = "ALARM TRIGGERED!";
                break;
        }
        String motionLine = (bikeMoving ? "MOVING " : "STOPPED ") + String(motionValueG, 2) + "g";
        String cloudLine = cloudOnline ? "Cloud: ONLINE" : "Cloud: OFFLINE";

        iotDisplayLines("Bike Monitor", stateLine, motionLine, cloudLine);
    }
}
