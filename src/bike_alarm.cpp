#include <Arduino.h>
#include "iot_pins.h"
#include "iot_common.h"
#include "bike_alarm.h"

void bikeAlarmBegin() {
    pinMode(IoTPins::LED_1, OUTPUT);
    pinMode(IoTPins::BUZZER, OUTPUT);
    
    // Ensure both are turned off initially
    digitalWrite(IoTPins::LED_1, LOW);
    noTone(IoTPins::BUZZER);
}

void bikeAlarmUpdate(bool bikeMoving) {
    static uint32_t lastToggleMs = 0;
    static bool alarmActive = false;

    if (bikeMoving) {
        // Toggle the LED and buzzer every 250 milliseconds
        if (iotEvery(250, lastToggleMs)) {
            alarmActive = !alarmActive;
            
            digitalWrite(IoTPins::LED_1, alarmActive ? HIGH : LOW);
            
            if (alarmActive) {
                tone(IoTPins::BUZZER, 1000);
            } else {
                noTone(IoTPins::BUZZER);
            }
        }
    } else {
        // Force the hardware to an off state when disarmed/stationary
        if (alarmActive) {
            digitalWrite(IoTPins::LED_1, LOW);
            noTone(IoTPins::BUZZER);
            alarmActive = false;
        }
    }
}