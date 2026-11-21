#include <Arduino.h>
#include "iot_pins.h"
#include "iot_common.h"
#include "bike_alarm.h"

#define MOTION_LOW  0.5
#define MOTION_HIGH 1.5

static constexpr uint8_t BUZZER_CHANNEL = 0;
static constexpr uint32_t BUZZER_FREQUENCY_HZ = 1000;
static constexpr uint8_t BUZZER_RESOLUTION_BITS = 8;
static constexpr uint32_t BUZZER_ON_DUTY = 1U << (BUZZER_RESOLUTION_BITS - 1);
static bool buzzerReady = false;

void bikeAlarmBegin() {
    pinMode(IoTPins::LED_1, OUTPUT);
    pinMode(IoTPins::LED_2, OUTPUT);
    pinMode(IoTPins::LED_3, OUTPUT);
    pinMode(IoTPins::BUZZER, OUTPUT);

    digitalWrite(IoTPins::LED_1, LOW);
    digitalWrite(IoTPins::LED_2, LOW);
    digitalWrite(IoTPins::LED_3, LOW);
    buzzerReady = ledcSetup(BUZZER_CHANNEL, BUZZER_FREQUENCY_HZ, BUZZER_RESOLUTION_BITS) != 0;
    if (buzzerReady) {
        ledcAttachPin(IoTPins::BUZZER, BUZZER_CHANNEL);
        ledcWrite(BUZZER_CHANNEL, 0);
    }
}

bool bikeAlarmUpdate(bool armed, float motionValueG) {
    static uint32_t lastToggleMs = 0;
    static bool alarmTriggered = false;
    static bool alarmActive = false;

    if (!armed) {
        if (buzzerReady && alarmActive) {
            ledcWrite(BUZZER_CHANNEL, 0);
        }
        alarmTriggered = false;
        alarmActive = false;

        digitalWrite(IoTPins::LED_1, LOW);
        digitalWrite(IoTPins::LED_2, LOW);
        digitalWrite(IoTPins::LED_3, LOW);
        return false;
    }

    if (motionValueG >= MOTION_HIGH) {
        alarmTriggered = true;
    }

    if (alarmTriggered) {
        if (iotEvery(250, lastToggleMs)) {
            alarmActive = !alarmActive;

            digitalWrite(IoTPins::LED_1, alarmActive ? HIGH : LOW);
            digitalWrite(IoTPins::LED_2, LOW);
            digitalWrite(IoTPins::LED_3, LOW);

            if (buzzerReady) {
                ledcWrite(BUZZER_CHANNEL, alarmActive ? BUZZER_ON_DUTY : 0);
            }
        }
    }

    else if (motionValueG >= MOTION_LOW) {
        digitalWrite(IoTPins::LED_1, LOW);
        digitalWrite(IoTPins::LED_2, HIGH);
        digitalWrite(IoTPins::LED_3, LOW);
    }

    return alarmTriggered;
}
