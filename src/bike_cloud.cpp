#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include "iot_common.h"
#include "iot_display.h"
#include "iot_provision.h"
#include "bike_cloud.h"

#define TEAM_ID "T04" // Update if needed

constexpr uint16_t MQTT_PORT = 8883;
constexpr uint16_t MQTT_KEEPALIVE_S = 15;
constexpr uint32_t WIFI_RETRY_MS = 20000;
constexpr uint32_t BACKOFF_START_MS = 1000;
constexpr uint32_t BACKOFF_MAX_MS = 30000;
constexpr uint32_t PERIOD_MS = 2000; // 2 s heartbeat + instant publish on state change

struct Credentials {
    String wifiSsid, wifiPass;
    String mqttHost;
    String mqttUser, mqttPass;
};

static Credentials creds;
static Preferences prefs;
static constexpr const char* NVS_NAMESPACE = "iot-lab5";

static WiFiClientSecure net;
static PubSubClient mqtt(net);

static const String TOPIC_TELEMETRY = String("iot/") + TEAM_ID + "/telemetry";
static const String TOPIC_STATUS    = String("iot/") + TEAM_ID + "/status";
static String clientId;

static uint32_t backoffMs = BACKOFF_START_MS;
static uint32_t lastMqttTry = 0;
static uint32_t seqNo = 0;
static uint32_t pubOk = 0, pubFail = 0, reconnects = 0;

// Latest sensor snapshot for Serial PUB command
static bool curArmed = true;
static bool curMoving = false;
static bool curAlarm = false;
static float curMotionG = 0.0f;

static bool loadCredentials() {
    if (!prefs.begin(NVS_NAMESPACE, true)) {
        return false;
    }
    creds.wifiSsid = prefs.getString("wifi_ssid", "Christopher_iPhone");
    creds.wifiPass = prefs.getString("wifi_pass", "HelloWorld");
    creds.mqttHost = prefs.getString("mqtt_host", "d511fce0.ala.us-east-1.emqxsl.com");
    creds.mqttUser = prefs.getString("mqtt_user", "User1");
    creds.mqttPass = prefs.getString("mqtt_pass", "Password");
    prefs.end();

    return creds.wifiSsid.length() > 0 &&
           creds.mqttHost.length() > 0 &&
           creds.mqttUser.length() > 0;
}

static void saveCredentials() {
    prefs.begin(NVS_NAMESPACE, false);
    prefs.putString("wifi_ssid", creds.wifiSsid);
    prefs.putString("wifi_pass", creds.wifiPass);
    prefs.putString("mqtt_host", creds.mqttHost);
    prefs.putString("mqtt_user", creds.mqttUser);
    prefs.putString("mqtt_pass", creds.mqttPass);
    prefs.end();
}

static void forgetCredentials() {
    prefs.begin(NVS_NAMESPACE, false);
    prefs.clear();
    prefs.end();
    creds = Credentials();
    Serial.println("Credentials erased from NVS.");
}

static void printCredentials() {
    Serial.print("  Wi-Fi SSID    : "); Serial.println(creds.wifiSsid);
    Serial.print("  Wi-Fi password: "); Serial.println(iotMasked(creds.wifiPass));
    Serial.print("  MQTT host     : "); Serial.println(creds.mqttHost);
    Serial.print("  MQTT username : "); Serial.println(creds.mqttUser);
    Serial.print("  MQTT password : "); Serial.println(iotMasked(creds.mqttPass));
}

static void runProvisioning() {
    Serial.println("\n=== Provisioning ===");
    do { creds.wifiSsid = iotAsk("Wi-Fi SSID:"); } while (creds.wifiSsid.length() == 0);
    creds.wifiPass = iotAsk("Wi-Fi password (leave empty for an open network):");
    do { creds.mqttHost = iotCleanHost(iotAsk("MQTT host (Cluster URL):")); } while (creds.mqttHost.length() == 0);
    do { creds.mqttUser = iotAsk("MQTT username:"); } while (creds.mqttUser.length() == 0);
    creds.mqttPass = iotAsk("MQTT password:");

    saveCredentials();
    Serial.println("Saved to NVS:");
    printCredentials();
}

static void configureMqtt() {
    // Encrypted TLS without broker certificate verification (Lab 5 setup)
    net.setInsecure();
    mqtt.setServer(creds.mqttHost.c_str(), MQTT_PORT);
    mqtt.setKeepAlive(MQTT_KEEPALIVE_S);
    mqtt.setSocketTimeout(5);
    mqtt.setBufferSize(512);
}

static bool mqttConnect() {
    Serial.print("MQTT: connecting to ");
    Serial.print(creds.mqttHost);
    Serial.print(" as ");
    Serial.println(clientId);

    bool connected = mqtt.connect(
        clientId.c_str(),
        creds.mqttUser.c_str(),
        creds.mqttPass.c_str(),
        TOPIC_STATUS.c_str(),
        1,
        true,
        "offline"
    );

    if (!connected) return false;

    if (!mqtt.publish(TOPIC_STATUS.c_str(), "online", true)) {
        mqtt.disconnect();
        return false;
    }

    Serial.println("MQTT: connected & published \"online\" (retained)");
    return true;
}

static void maintainWiFi() {
    static uint32_t lastBegin = 0;
    static bool started = false, wasUp = false;
    const bool up = (WiFi.status() == WL_CONNECTED);

    if (up && !wasUp) {
        Serial.print("Wi-Fi: connected, IP ");
        Serial.println(WiFi.localIP());
    }
    if (!up && wasUp) {
        Serial.println("Wi-Fi: connection lost");
    }
    wasUp = up;

    if (!up && (!started || millis() - lastBegin >= WIFI_RETRY_MS)) {
        started = true;
        lastBegin = millis();
        Serial.print("Wi-Fi: connecting to ");
        Serial.println(creds.wifiSsid);
        WiFi.begin(creds.wifiSsid.c_str(), creds.wifiPass.c_str());
    }
}

static void maintainMqtt() {
    if (WiFi.status() != WL_CONNECTED || mqtt.connected()) return;
    if (millis() - lastMqttTry < backoffMs) return;
    lastMqttTry = millis();

    if (mqttConnect()) {
        backoffMs = BACKOFF_START_MS;
        reconnects++;
    } else {
        Serial.printf("MQTT: connect failed (rc=%d)\n", mqtt.state());
        backoffMs = min((unsigned long)backoffMs * 2, (unsigned long)BACKOFF_MAX_MS) + (millis() % 500);
    }
}

static bool publishTelemetry() {
    if (!mqtt.connected()) return false;

    char buf[224];
    int n = snprintf(
        buf, sizeof(buf),
        "{\"seq\":%lu,\"armed\":%s,\"moving\":%s,\"alarm\":%s,\"motion_g\":%.2f,\"rssi\":%d,\"uptime_s\":%lu}",
        (unsigned long)++seqNo,
        curArmed ? "true" : "false",
        curMoving ? "true" : "false",
        curAlarm ? "true" : "false",
        curMotionG,
        WiFi.RSSI(),
        (unsigned long)(millis() / 1000)
    );

    if (n < 0 || n >= (int)sizeof(buf)) return false;

    bool ok = mqtt.publish(TOPIC_TELEMETRY.c_str(), buf, false);
    if (ok) {
        Serial.printf("PUB %s  %s\n", TOPIC_TELEMETRY.c_str(), buf);
    }
    return ok;
}

static void goOfflineCleanly() {
    if (mqtt.connected()) {
        mqtt.publish(TOPIC_STATUS.c_str(), "offline", true);
        mqtt.disconnect();
    }
}

static void handleCommand(String line) {
    line.trim();
    line.toUpperCase();
    if (line.length() == 0) return;

    if (line == "HELP") {
        Serial.println("Commands: HELP | STATUS | PUB | PROVISION | FORGET");
    } else if (line == "STATUS") {
        Serial.printf("Team %s | Wi-Fi: %s | MQTT: %s | Pub OK/Fail: %lu/%lu\n",
                      TEAM_ID,
                      WiFi.status() == WL_CONNECTED ? "up" : "down",
                      mqtt.connected() ? "online" : "offline",
                      (unsigned long)pubOk, (unsigned long)pubFail);
    } else if (line == "PUB") {
        if (publishTelemetry()) pubOk++; else pubFail++;
    } else if (line == "FORGET") {
        goOfflineCleanly();
        forgetCredentials();
        ESP.restart();
    } else if (line == "PROVISION") {
        goOfflineCleanly();
        runProvisioning();
        ESP.restart();
    }
}

static void pollConsole() {
    static String buf;
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\r') continue;
        if (c == '\n') {
            handleCommand(buf);
            buf = "";
        } else if (buf.length() < 60) {
            buf += c;
        }
    }
}

void bikeCloudBegin() {
    uint64_t chip = ESP.getEfuseMac();
    char idbuf[32];
    snprintf(idbuf, sizeof(idbuf), "iot-%s-%08lX", TEAM_ID, (unsigned long)(chip & 0xFFFFFFFF));
    clientId = idbuf;

    Serial.println("Type PROVISION or FORGET within 3 s...");
    iotDisplayLines("Bike Cloud", "PROVISION/FORGET?", "(3 s, Serial)");
    String cmd = iotReadLine(3000);
    cmd.toUpperCase();
    if (cmd == "FORGET") forgetCredentials();

    bool haveCreds = loadCredentials();
    if (cmd == "PROVISION" || !haveCreds) {
        iotDisplayLines("Bike Cloud", "Provisioning:", "use Serial Monitor");
        runProvisioning();
    } else {
        Serial.println("Credentials loaded from NVS:");
        printCredentials();
    }

    WiFi.mode(WIFI_STA);
    configureMqtt();
}

bool bikeCloudUpdate(bool armed, bool bikeMoving, bool alarmTriggered, float motionValueG) {
    static bool prevArmed = true;
    static bool prevMoving = false;
    static bool prevAlarm = false;
    static uint32_t lastPub = 0;

    curArmed = armed;
    curMoving = bikeMoving;
    curAlarm = alarmTriggered;
    curMotionG = motionValueG;

    pollConsole();
    maintainWiFi();
    maintainMqtt();
    mqtt.loop();

    bool stateChanged = (armed != prevArmed) || (bikeMoving != prevMoving) || (alarmTriggered != prevAlarm);
    if (stateChanged) {
        prevArmed = armed;
        prevMoving = bikeMoving;
        prevAlarm = alarmTriggered;
    }

    // Publish immediately on state change OR on regular PERIOD_MS timer
    if (mqtt.connected() && (stateChanged || iotEvery(PERIOD_MS, lastPub))) {
        if (publishTelemetry()) {
            pubOk++;
            lastPub = millis(); // reset interval timer after an immediate state publish
        } else {
            pubFail++;
        }
    }

    return mqtt.connected();
}