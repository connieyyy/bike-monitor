#include <Arduino.h>
#include "iot_pins.h"
#include "bike_button.h"

static constexpr uint32_t DEBOUNCE_MS = 50;

static bool armed = true;
static bool lastButtonState = HIGH;
static uint32_t lastDebounceMs = 0;

void bikeButtonBegin() {
    pinMode(IoTPins::BUTTON_1, INPUT_PULLUP);
    Serial.println("Button disarm ready.");
}

bool bikeButtonUpdate() {
    bool reading = digitalRead(IoTPins::BUTTON_1);

    if (reading != lastButtonState) {
        lastDebounceMs = millis();
    }
    lastButtonState = reading;

    static bool prevStable = HIGH;
    if ((millis() - lastDebounceMs) > DEBOUNCE_MS) {
        if (reading != prevStable) {
            prevStable = reading;

            if (reading == LOW) {
                armed = !armed;
                Serial.print("BUTTON: System ");
                Serial.println(armed ? "ARMED" : "DISARMED");
            }
        }
    }

    return armed;
}
