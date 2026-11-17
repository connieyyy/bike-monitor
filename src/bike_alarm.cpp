#include <Arduino.h>
#include "iot_pins.h"
#include "iot_common.h"
#include "bike_alarm.h"

#define MOTION_LOW  0.5
#define MOTION_HIGH 1.5

void bikeAlarmBegin() {
    pinMode(IoTPins::LED_1, OUTPUT);
    pinMode(IoTPins::LED_2, OUTPUT);
    pinMode(IoTPins::LED_3, OUTPUT);
    pinMode(IoTPins::BUZZER, OUTPUT);

    digitalWrite(IoTPins::LED_1, LOW);
    digitalWrite(IoTPins::LED_2, LOW);
    digitalWrite(IoTPins::LED_3, LOW);
    noTone(IoTPins::BUZZER);
}

bool bikeAlarmUpdate(bool armed, float motionValue) {
    static uint32_t lastToggleMs = 0;
    static bool alarmTriggered = false;
    static bool alarmActive = false;

    if (!armed) {
        alarmTriggered = false;
        alarmActive = false;

        digitalWrite(IoTPins::LED_1, LOW);
        digitalWrite(IoTPins::LED_2, LOW);
        digitalWrite(IoTPins::LED_3, LOW);
        noTone(IoTPins::BUZZER);

        return false;
    }

    if (motionValue >= MOTION_HIGH) {
        alarmTriggered = true;
    }

    if (alarmTriggered) {
        if (iotEvery(250, lastToggleMs)) {
            alarmActive = !alarmActive;

            digitalWrite(IoTPins::LED_1, alarmActive ? HIGH : LOW);
            digitalWrite(IoTPins::LED_2, LOW);
            digitalWrite(IoTPins::LED_3, LOW);

            if (alarmActive) {
                tone(IoTPins::BUZZER, 1000);
            } else {
                noTone(IoTPins::BUZZER);
            }
        }
    }

    else if (motionValue >= MOTION_LOW) {
        digitalWrite(IoTPins::LED_1, LOW);
        digitalWrite(IoTPins::LED_2, HIGH);
        digitalWrite(IoTPins::LED_3, LOW);
        noTone(IoTPins::BUZZER);
    }

    return alarmTriggered;
}