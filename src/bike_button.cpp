#include <Arduino.h>
#include "iot_pins.h"
#include "bike_button.h"

static constexpr uint32_t DEBOUNCE_MS = 50;
static constexpr uint32_t SEQUENCE_TIMEOUT_MS = 3000;

struct ButtonTracker {
    uint8_t pin;
    bool lastState;
    bool prevStable;
    uint32_t lastDebounceMs;
};

static bool armed = true;
static uint8_t sequenceStep = 0;
static uint32_t sequenceStartMs = 0;

static ButtonTracker btn1 = { IoTPins::BUTTON_1, HIGH, HIGH, 0 };
static ButtonTracker btn2 = { IoTPins::BUTTON_2, HIGH, HIGH, 0 };
static ButtonTracker btn3 = { IoTPins::BUTTON_3, HIGH, HIGH, 0 };

static bool wasButtonPressed(ButtonTracker& btn) {
    bool reading = digitalRead(btn.pin);

    if (reading != btn.lastState) {
        btn.lastDebounceMs = millis();
    }
    btn.lastState = reading;

    if ((millis() - btn.lastDebounceMs) > DEBOUNCE_MS) {
        if (reading != btn.prevStable) {
            btn.prevStable = reading;
            if (reading == LOW) {
                return true;
            }
        }
    }
    return false;
}

void bikeButtonBegin() {
    pinMode(IoTPins::BUTTON_1, INPUT_PULLUP);
    pinMode(IoTPins::BUTTON_2, INPUT_PULLUP);
    pinMode(IoTPins::BUTTON_3, INPUT_PULLUP);
    Serial.println("3-button arm/disarm ready.");
}

bool bikeButtonUpdate(bool bikeMoving, bool alarmTriggered) {
    bool b1Pressed = wasButtonPressed(btn1);
    bool b2Pressed = wasButtonPressed(btn2);
    bool b3Pressed = wasButtonPressed(btn3);

    bool isStationary = (!bikeMoving && !alarmTriggered);

    // Reset the Button 2 -> Button 3 sequence if it takes too long
    if (sequenceStep == 1 && (millis() - sequenceStartMs) > SEQUENCE_TIMEOUT_MS) {
        sequenceStep = 0;
        Serial.println("SEQUENCE: Timed out.");
    }

    // Button 1: Arm anytime, or disarm ONLY when stationary and alarm is not active
    if (b1Pressed) {
        sequenceStep = 0;
        if (!armed) {
            armed = true;
            Serial.println("BUTTON 1: System ARMED");
        } else if (isStationary) {
            armed = false;
            Serial.println("BUTTON 1: System DISARMED (stationary)");
        } else {
            Serial.println("BUTTON 1: Cannot disarm while active/moving. Press Button 2 -> Button 3.");
        }
    }

    // Button 2: First step of the active disarm sequence
    if (b2Pressed && armed) {
        sequenceStep = 1;
        sequenceStartMs = millis();
        Serial.println("BUTTON 2: Sequence started (press Button 3 within 3s)...");
    }

    // Button 3: Second step of the active disarm sequence
    if (b3Pressed && armed) {
        if (sequenceStep == 1) {
            armed = false;
            sequenceStep = 0;
            Serial.println("BUTTON 3: Sequence complete! System DISARMED");
        } else {
            sequenceStep = 0;
            Serial.println("BUTTON 3: Wrong order (press Button 2 first).");
        }
    }

    return armed;
}