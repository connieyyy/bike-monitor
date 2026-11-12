#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "iot_common.h"
#include "bike_motion.h"

static constexpr float MOTION_THRESHOLD_G = 1.3f;
static constexpr uint32_t SAMPLE_INTERVAL_MS = 100;
static constexpr uint32_t MOTION_HOLD_MS = 2000;

static Adafruit_MPU6050 mpu;
static bool mpuReady = false;

bool bikeMotionBegin() {
    if (!mpu.begin(0x68, &Wire)) {
        Serial.println("ERROR: MPU-6050 not found at 0x68.");
        return false;
    }

    mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

    Serial.println("MPU-6050 ready.");
    mpuReady = true;
    return true;
}

void bikeMotionUpdate(bool& bikeMoving, float& motionValueG) {
    static uint32_t lastSampleMs = 0;
    static uint32_t lastMotionMs = 0;

    if (!mpuReady) {
        bikeMoving = false;
        motionValueG = 0.0f;
        return;
    }

    if (!iotEvery(SAMPLE_INTERVAL_MS, lastSampleMs)) {
        return;
    }

    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);

    float ax = accel.acceleration.x / 9.81f;
    float ay = accel.acceleration.y / 9.81f;
    float az = accel.acceleration.z / 9.81f;
    motionValueG = sqrtf(ax * ax + ay * ay + az * az);

    if (motionValueG > MOTION_THRESHOLD_G) {
        lastMotionMs = millis();
    }

    // Hold "moving" for MOTION_HOLD_MS after the last spike
    bikeMoving = (millis() - lastMotionMs) < MOTION_HOLD_MS;
}
