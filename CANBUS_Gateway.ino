/*
 * CANIMEX Gateway - ESP32 CAN X2
 *
 * Arduino IDE version
 *
 * IMPORTANT: For Arduino IDE users:
 * 1. Install ESP32 board support
 * 2. Copy all .h files from include/ to the sketch folder
 * 3. Copy all .cpp files from src/ (except main.cpp) to the sketch folder
 * 4. Select "ESP32 Dev Module" as board
 * 5. Set upload speed to 921600
 *
 * For PlatformIO users: Use src/main.cpp instead
 */

#include <Arduino.h>
#include "WiFiConfig.h"
#include "CANConfig.h"
#include "CANopen.h"
#include "Menu.h"

// Global objects
WiFiConfig wifiConfig;
CANConfig canConfig;
CANopen* canopen0 = nullptr;
CANopen* canopen1 = nullptr;
Menu* menu = nullptr;

// Timing variables
unsigned long lastSnifferUpdate = 0;
unsigned long lastHeartbeat = 0;
const unsigned long SNIFFER_INTERVAL = 10;  // 10ms
const unsigned long HEARTBEAT_INTERVAL = 1000;  // 1 second

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n========================================");
    Serial.println("     CANIMEX GATEWAY v1.0");
    Serial.println("     ESP32 CAN X2 - Dual CAN Bus");
    Serial.println("========================================\n");

    // Initialize WiFi
    Serial.println("Initializing WiFi...");
    wifiConfig.begin();
    delay(500);

    // Initialize CAN bus
    Serial.println("Initializing CAN bus...");
    canConfig.begin();
    delay(500);

    // Initialize CANopen for both CAN channels
    Serial.println("Initializing CANopen...");
    canopen0 = new CANopen(canConfig.getCAN0());
    canopen1 = new CANopen(canConfig.getCAN1());
    canopen0->begin();
    canopen1->begin();

    // Initialize menu system
    Serial.println("Initializing menu system...");
    menu = new Menu(&wifiConfig, &canConfig, canopen0, canopen1);
    menu->begin();

    Serial.println("\n========================================");
    Serial.println("    System Ready!");
    Serial.println("========================================\n");

    // Display initial status
    wifiConfig.printStatus();
    canConfig.printStatus();
}

void loop() {
    // Process menu input
    menu->process();

    // Process CANopen messages
    canopen0->process();
    canopen1->process();

    // Handle CAN bus sniffing
    if (canConfig.isSniffing()) {
        if (millis() - lastSnifferUpdate >= SNIFFER_INTERVAL) {
            lastSnifferUpdate = millis();

            CANMessage msg;
            CANBusChannel* channel = (canConfig.getSniffingChannel() == 0)
                                    ? canConfig.getCAN0()
                                    : canConfig.getCAN1();

            if (channel->receiveMessage(msg, 0)) {
                // Print the message
                Serial.print("[");
                Serial.print(millis());
                Serial.print("] ");
                channel->printMessage(msg);
                Serial.println();
            }
        }
    }

    // Send periodic heartbeat (for CANopen nodes)
    if (millis() - lastHeartbeat >= HEARTBEAT_INTERVAL) {
        lastHeartbeat = millis();
        // Heartbeat can be enabled if needed
        // canopen0->sendHeartbeat(1, 0x05);  // Node 1, operational state
    }

    // Small delay to prevent watchdog issues
    delay(1);
}
