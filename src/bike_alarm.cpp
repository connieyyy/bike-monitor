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

void bikeAlarmUpdate(bool armed, bool bikeMoving) {
    static uint32_t lastToggleMs = 0;
    static bool alarmTriggered = false;
    static bool alarmActive = false;

    // Reset the latched alarm and shut off hardware when disarmed
    if (!armed) {
        alarmTriggered = false;
        if (alarmActive) {
            digitalWrite(IoTPins::LED_1, LOW);
            noTone(IoTPins::BUZZER);
            alarmActive = false;
        }
        return;
    }

    // Latch the alarm once movement is detected while armed
    if (bikeMoving && !alarmTriggered) {
        alarmTriggered = true;
    }

    // Keep pulsing the LED and buzzer continuously until disarmed
    if (alarmTriggered) {
        if (iotEvery(250, lastToggleMs)) {
            alarmActive = !alarmActive;
            
            digitalWrite(IoTPins::LED_1, alarmActive ? HIGH : LOW);
            
            if (alarmActive) {
                tone(IoTPins::BUZZER, 1000);
            } else {
                noTone(IoTPins::BUZZER);
            }
        }
    }
}