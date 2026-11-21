#include <Arduino.h>
#include "iot_pins.h"
#include "bike_alarm.h"

static constexpr float MOTION_LOW_G = 0.5f;
static constexpr float MOTION_HIGH_G = 1.5f;
static constexpr uint32_t WARNING_DURATION_MS = 3000;
static constexpr uint32_t FLASH_INTERVAL_MS = 250;

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

BikeAlarmState bikeAlarmUpdate(bool armed, bool bikeMoving, float motionValueG) {
    static BikeAlarmState state = BikeAlarmState::Clear;
    static uint32_t stateStartedMs = 0;
    static bool buzzerOn = false;

    const uint32_t now = millis();
    if (!armed) {
        state = BikeAlarmState::Clear;
    } else {
        switch (state) {
            case BikeAlarmState::Clear:
                if (motionValueG >= MOTION_HIGH_G) {
                    state = BikeAlarmState::Warning;
                    stateStartedMs = now;
                }
                break;
            case BikeAlarmState::Warning:
                if (now - stateStartedMs >= WARNING_DURATION_MS) {
                    state = bikeMoving ? BikeAlarmState::Triggered : BikeAlarmState::Clear;
                    stateStartedMs = now;
                }
                break;
            case BikeAlarmState::Triggered:
                break;
        }
    }

    const bool flashOn = ((now - stateStartedMs) / FLASH_INTERVAL_MS) % 2 == 0;
    const bool redOn = state == BikeAlarmState::Triggered && flashOn;
    const bool warningLight = state == BikeAlarmState::Warning && flashOn;
    const bool armedLight = armed && state == BikeAlarmState::Clear && motionValueG >= MOTION_LOW_G;
    digitalWrite(IoTPins::LED_1, redOn ? HIGH : LOW);
    digitalWrite(IoTPins::LED_2, (warningLight || armedLight) ? HIGH : LOW);

    if (buzzerReady && redOn != buzzerOn) {
        ledcWrite(BUZZER_CHANNEL, redOn ? BUZZER_ON_DUTY : 0);
        buzzerOn = redOn;
    }
    return state;
}
