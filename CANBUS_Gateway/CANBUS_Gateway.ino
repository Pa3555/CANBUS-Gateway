/*
 * CANIMEX Gateway - ESP32-CAN-X2 from Autosport Labs
 *
 * Dual CAN bus gateway with WiFi AP/Client mode and CANopen joystick support
 *
 * Hardware: ESP32-CAN-X2 (ESP32-S3)
 * - CAN1: Built-in TWAI controller (GPIO6/GPIO7)
 * - CAN2: MCP2515 via SPI
 *
 * Default WiFi AP:
 *   SSID: CANIMEX_GATEWAY
 *   Password: Canimex2026
 *   IP: 192.168.4.1
 *
 * Arduino IDE Setup:
 * 1. Install ESP32 board support: https://dl.espressif.com/dl/package_esp32_index.json
 * 2. Select Board: "AutosportLabs ESP32-CAN-X2"
 * 3. Install library: "mcp_canbus" by Longan Labs
 * 4. Upload!
 */

#include <WiFi.h>
#include <Preferences.h>
#include <driver/gpio.h>
#include <driver/twai.h>
#include <mcp_can.h>
#include <SPI.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include "web_interface.h"

// ============================================
// PIN DEFINITIONS FOR ESP32-CAN-X2
// ============================================
// CAN1 - Built-in TWAI Controller
#define CAN1_TX_PIN GPIO_NUM_7
#define CAN1_RX_PIN GPIO_NUM_6

// CAN2 - MCP2515 via SPI (Custom SPI pins)
#define MCP2515_CS   10  // Chip Select
#define MCP2515_MOSI 11  // Master Out Slave In
#define MCP2515_CLK  12  // Clock
#define MCP2515_MISO 13  // Master In Slave Out
#define MCP2515_IRQ  3   // Interrupt

// LED
#define LED_BUILTIN 2

// ============================================
// CONFIGURATION STRUCTURES
// ============================================

enum GatewayWiFiMode {
    GATEWAY_WIFI_AP = 0,
    GATEWAY_WIFI_CLIENT = 1
};

enum CANSpeed_t {
    CAN_SPEED_125KBPS = 0,
    CAN_SPEED_250KBPS = 1,
    CAN_SPEED_500KBPS = 2,
    CAN_SPEED_1000KBPS = 3
};

// CAN Message structure for queue
struct CANMessage {
    uint8_t channel;     // 1 or 2
    uint32_t id;
    uint8_t len;
    uint8_t data[8];
    uint32_t timestamp;
};

// ============================================
// GLOBAL OBJECTS
// ============================================

Preferences prefs;
MCP_CAN CAN2(MCP2515_CS);  // CAN2 via MCP2515
AsyncWebServer server(80);  // Web server on port 80
AsyncWebSocket ws("/ws");   // WebSocket endpoint

// Configuration variables
GatewayWiFiMode wifiMode = GATEWAY_WIFI_AP;
String apSSID = "CANIMEX_GATEWAY";
String apPassword = "Canimex2026";
String clientSSID = "";
String clientPassword = "";
CANSpeed_t can1Speed = CAN_SPEED_500KBPS;
CANSpeed_t can2Speed = CAN_SPEED_500KBPS;

bool can1Running = false;
bool can2Running = false;
bool sniffingEnabled = false;
uint8_t sniffingChannel = 0;
bool webMonitoringEnabled = false;

unsigned long lastHeartbeat = 0;
unsigned long can1MessageCount = 0;
unsigned long can2MessageCount = 0;

// ============================================
// DUAL-CORE & PSRAM CONFIGURATION
// ============================================

// FreeRTOS Queues for inter-core communication
QueueHandle_t canToWebQueue = NULL;    // CAN messages to WebSocket
QueueHandle_t serialOutputQueue = NULL; // Messages for serial output

// Task handles
TaskHandle_t canTaskHandle = NULL;
TaskHandle_t networkTaskHandle = NULL;

// Mutex for shared resources
SemaphoreHandle_t configMutex = NULL;

// PSRAM statistics
size_t psramTotal = 0;
size_t psramUsed = 0;
bool psramAvailable = false;

// Core statistics
uint32_t core0IdleTime = 0;
uint32_t core1IdleTime = 0;
float core0Load = 0.0;
float core1Load = 0.0;

// ============================================
// CAN SPEED CONFIGURATION
// ============================================

twai_timing_config_t getCANTimingConfig(CANSpeed_t speed) {
    switch (speed) {
        case CAN_SPEED_125KBPS:
            return TWAI_TIMING_CONFIG_125KBITS();
        case CAN_SPEED_250KBPS:
            return TWAI_TIMING_CONFIG_250KBITS();
        case CAN_SPEED_1000KBPS:
            return TWAI_TIMING_CONFIG_1MBITS();
        case CAN_SPEED_500KBPS:
        default:
            return TWAI_TIMING_CONFIG_500KBITS();
    }
}

uint8_t getMCP2515Speed(CANSpeed_t speed) {
    switch (speed) {
        case CAN_SPEED_125KBPS:
            return CAN_125KBPS;
        case CAN_SPEED_250KBPS:
            return CAN_250KBPS;
        case CAN_SPEED_1000KBPS:
            return CAN_1000KBPS;
        case CAN_SPEED_500KBPS:
        default:
            return CAN_500KBPS;
    }
}

// ============================================
// WIFI FUNCTIONS
// ============================================

void loadConfig() {
    prefs.begin("gateway", false);

    wifiMode = (GatewayWiFiMode)prefs.getUChar("wifi_mode", GATEWAY_WIFI_AP);
    apSSID = prefs.getString("ap_ssid", "CANIMEX_GATEWAY");
    apPassword = prefs.getString("ap_pass", "Canimex2026");
    clientSSID = prefs.getString("cli_ssid", "");
    clientPassword = prefs.getString("cli_pass", "");
    can1Speed = (CANSpeed_t)prefs.getUChar("can1_speed", CAN_SPEED_500KBPS);
    can2Speed = (CANSpeed_t)prefs.getUChar("can2_speed", CAN_SPEED_500KBPS);

    Serial.println("Configuration loaded from flash");
}

void saveConfig() {
    prefs.putUChar("wifi_mode", wifiMode);
    prefs.putString("ap_ssid", apSSID);
    prefs.putString("ap_pass", apPassword);
    prefs.putString("cli_ssid", clientSSID);
    prefs.putString("cli_pass", clientPassword);
    prefs.putUChar("can1_speed", can1Speed);
    prefs.putUChar("can2_speed", can2Speed);

    Serial.println("Configuration saved to flash");
}

void startWiFiAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID.c_str(), apPassword.c_str());

    Serial.println("\n=== Access Point Started ===");
    Serial.print("SSID: ");
    Serial.println(apSSID);
    Serial.print("Password: ");
    Serial.println(apPassword);
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("============================\n");
}

void startWiFiClient() {
    if (clientSSID.length() == 0) {
        Serial.println("ERROR: Client SSID not configured!");
        Serial.println("Falling back to AP mode...");
        wifiMode = GATEWAY_WIFI_AP;
        startWiFiAP();
        return;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(clientSSID.c_str(), clientPassword.c_str());

    Serial.print("Connecting to WiFi: ");
    Serial.println(clientSSID);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n=== WiFi Client Connected ===");
        Serial.print("SSID: ");
        Serial.println(clientSSID);
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.println("=============================\n");
    } else {
        Serial.println("\nERROR: Failed to connect to WiFi!");
        Serial.println("Falling back to AP mode...");
        wifiMode = GATEWAY_WIFI_AP;
        startWiFiAP();
    }
}

// ============================================
// CAN1 FUNCTIONS (TWAI)
// ============================================

bool startCAN1() {
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN1_TX_PIN, CAN1_RX_PIN, TWAI_MODE_NORMAL);
    g_config.tx_queue_len = 20;
    g_config.rx_queue_len = 20;

    twai_timing_config_t t_config = getCANTimingConfig(can1Speed);
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        if (twai_start() == ESP_OK) {
            can1Running = true;
            Serial.println("CAN1: Started successfully");
            return true;
        }
    }

    Serial.println("ERROR: Failed to start CAN1");
    return false;
}

void stopCAN1() {
    if (can1Running) {
        twai_stop();
        twai_driver_uninstall();
        can1Running = false;
        Serial.println("CAN1: Stopped");
    }
}

bool sendCAN1(uint32_t id, uint8_t* data, uint8_t len) {
    if (!can1Running) return false;

    twai_message_t message;
    message.identifier = id;
    message.data_length_code = len;
    message.flags = TWAI_MSG_FLAG_NONE;

    for (int i = 0; i < len && i < 8; i++) {
        message.data[i] = data[i];
    }

    return (twai_transmit(&message, pdMS_TO_TICKS(100)) == ESP_OK);
}

bool receiveCAN1(uint32_t& id, uint8_t* data, uint8_t& len) {
    if (!can1Running) return false;

    twai_message_t message;
    if (twai_receive(&message, 0) == ESP_OK) {
        id = message.identifier;
        len = message.data_length_code;

        for (int i = 0; i < len && i < 8; i++) {
            data[i] = message.data[i];
        }

        return true;
    }

    return false;
}

// ============================================
// CAN2 FUNCTIONS (MCP2515)
// ============================================

bool startCAN2() {
    // Configure SPI with custom pins for MCP2515
    SPI.begin(MCP2515_CLK, MCP2515_MISO, MCP2515_MOSI, MCP2515_CS);

    uint8_t mcp2515_speed = getMCP2515Speed(can2Speed);

    if (CAN2.begin(MCP_ANY, mcp2515_speed, MCP_8MHZ) == CAN_OK) {
        CAN2.setMode(MCP_NORMAL);
        can2Running = true;
        Serial.println("CAN2: Started successfully");
        return true;
    }

    Serial.println("ERROR: Failed to start CAN2 (MCP2515)");
    return false;
}

bool sendCAN2(uint32_t id, uint8_t* data, uint8_t len) {
    if (!can2Running) return false;

    return (CAN2.sendMsgBuf(id, 0, len, data) == CAN_OK);
}

bool receiveCAN2(uint32_t& id, uint8_t* data, uint8_t& len) {
    if (!can2Running) return false;

    if (CAN2.checkReceive() == CAN_MSGAVAIL) {
        CAN2.readMsgBuf(&id, &len, data);
        return true;
    }

    return false;
}

// ============================================
// WEB SERVER FUNCTIONS
// ============================================

void sendCANMessageToWeb(uint8_t channel, uint32_t id, uint8_t* data, uint8_t len) {
    if (!webMonitoringEnabled || ws.count() == 0) return;

    StaticJsonDocument<256> doc;
    doc["type"] = "can";
    doc["channel"] = channel;

    char idStr[9];
    sprintf(idStr, "%08X", id);
    doc["id"] = idStr;
    doc["dlc"] = len;

    String dataStr = "";
    for (int i = 0; i < len; i++) {
        char hex[4];
        sprintf(hex, "%02X ", data[i]);
        dataStr += hex;
    }
    doc["data"] = dataStr;

    String jsonString;
    serializeJson(doc, jsonString);
    ws.textAll(jsonString);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        // Handle incoming WebSocket messages if needed
    }
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
                       void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected\n", client->id());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void setupWebServer() {
    // WebSocket handler
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);

    // Serve main page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
    });

    // API: Get status
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        StaticJsonDocument<768> doc;
        doc["type"] = "status";
        doc["ip"] = (wifiMode == GATEWAY_WIFI_AP) ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
        doc["wifi_mode"] = (wifiMode == GATEWAY_WIFI_AP) ? "Access Point" : "Client";
        doc["wifi_mode_val"] = wifiMode;

        const char* speedNames[] = {"125 kbps", "250 kbps", "500 kbps", "1000 kbps"};
        doc["can1_speed"] = speedNames[can1Speed];
        doc["can2_speed"] = speedNames[can2Speed];
        doc["can1_speed_val"] = can1Speed;
        doc["can2_speed_val"] = can2Speed;
        doc["can1_running"] = can1Running;
        doc["can2_running"] = can2Running;
        doc["can1_count"] = can1MessageCount;
        doc["can2_count"] = can2MessageCount;

        unsigned long uptime = millis() / 1000;
        char uptimeStr[32];
        sprintf(uptimeStr, "%luh %lum %lus", uptime / 3600, (uptime % 3600) / 60, uptime % 60);
        doc["uptime"] = uptimeStr;

        // PSRAM stats
        doc["psram_available"] = psramAvailable;
        if (psramAvailable) {
            doc["psram_total"] = psramTotal;
            doc["psram_used"] = psramUsed;
            doc["psram_free"] = psramTotal - psramUsed;
            doc["psram_usage_pct"] = (psramUsed * 100.0) / psramTotal;
        }

        // Core stats
        doc["core0_load"] = core0Load;
        doc["core1_load"] = core1Load;

        // Free heap
        doc["heap_free"] = ESP.getFreeHeap();
        doc["heap_total"] = ESP.getHeapSize();

        String jsonString;
        serializeJson(doc, jsonString);
        request->send(200, "application/json", jsonString);
    });

    // API: Save WiFi config
    server.on("/api/wifi", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            StaticJsonDocument<256> doc;
            DeserializationError error = deserializeJson(doc, data);

            if (!error) {
                GatewayWiFiMode newMode = (GatewayWiFiMode)(int)doc["mode"];
                String ssid = doc["ssid"].as<String>();
                String pass = doc["password"].as<String>();

                if (newMode == GATEWAY_WIFI_AP && ssid.length() > 0) {
                    apSSID = ssid;
                    apPassword = pass;
                } else if (newMode == GATEWAY_WIFI_CLIENT && ssid.length() > 0) {
                    clientSSID = ssid;
                    clientPassword = pass;
                }

                wifiMode = newMode;
                saveConfig();

                request->send(200, "application/json", "{\"success\":true,\"message\":\"WiFi config saved. Restart to apply.\"}");
            } else {
                request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
            }
        });

    // API: Save CAN config
    server.on("/api/can", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            StaticJsonDocument<128> doc;
            DeserializationError error = deserializeJson(doc, data);

            if (!error) {
                CANSpeed_t newCAN1Speed = (CANSpeed_t)(int)doc["can1_speed"];
                CANSpeed_t newCAN2Speed = (CANSpeed_t)(int)doc["can2_speed"];

                if (newCAN1Speed != can1Speed) {
                    can1Speed = newCAN1Speed;
                    stopCAN1();
                    startCAN1();
                }

                if (newCAN2Speed != can2Speed) {
                    can2Speed = newCAN2Speed;
                    can2Running = false;
                    startCAN2();
                }

                saveConfig();
                request->send(200, "application/json", "{\"success\":true,\"message\":\"CAN config updated\"}");
            } else {
                request->send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
            }
        });

    // API: Toggle monitoring
    server.on("/api/monitor", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            StaticJsonDocument<64> doc;
            DeserializationError error = deserializeJson(doc, data);

            if (!error) {
                webMonitoringEnabled = doc["enabled"];
                request->send(200, "application/json", "{\"success\":true}");
            } else {
                request->send(400, "application/json", "{\"success\":false}");
            }
        });

    // API: Reset to defaults
    server.on("/api/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Resetting...\"}");
        delay(100);
        prefs.clear();
        ESP.restart();
    });

    server.begin();
    Serial.println("Web server started");
    Serial.print("Access at: http://");
    Serial.println((wifiMode == GATEWAY_WIFI_AP) ? WiFi.softAPIP().toString() : WiFi.localIP().toString());
}

// ============================================
// MENU SYSTEM
// ============================================

void displayMenu() {
    Serial.println("\n========== MAIN MENU ==========");
    Serial.println("1. WiFi Configuration");
    Serial.println("2. CAN Bus Configuration");
    Serial.println("3. CAN Sniffer");
    Serial.println("4. View Status");
    Serial.println("5. Reset to Defaults");
    Serial.println("h. Show this menu");
    Serial.println("===============================");
    Serial.print("\nEnter choice: ");
}

void handleMenu() {
    if (!Serial.available()) return;

    char choice = Serial.read();
    while (Serial.available()) Serial.read(); // Clear buffer

    if (choice == '\n' || choice == '\r') return;

    Serial.println(choice);

    switch (choice) {
        case '1':
            wifiMenu();
            break;
        case '2':
            canMenu();
            break;
        case '3':
            snifferMenu();
            break;
        case '4':
            printStatus();
            displayMenu();
            break;
        case '5':
            resetConfig();
            break;
        case 'h':
        case 'H':
            displayMenu();
            break;
        default:
            Serial.println("Invalid choice!");
            displayMenu();
            break;
    }
}

void wifiMenu() {
    Serial.println("\n===== WiFi Configuration =====");
    Serial.println("1. Switch to AP Mode");
    Serial.println("2. Switch to Client Mode");
    Serial.println("3. Configure AP Credentials");
    Serial.println("4. Configure Client Credentials");
    Serial.println("5. Back to Main Menu");
    Serial.print("\nEnter choice: ");

    while (!Serial.available()) delay(10);
    char choice = Serial.read();
    while (Serial.available()) Serial.read();
    Serial.println(choice);

    switch (choice) {
        case '1':
            wifiMode = GATEWAY_WIFI_AP;
            saveConfig();
            Serial.println("Restarting in AP mode...");
            delay(1000);
            ESP.restart();
            break;
        case '2':
            wifiMode = GATEWAY_WIFI_CLIENT;
            saveConfig();
            Serial.println("Restarting in Client mode...");
            delay(1000);
            ESP.restart();
            break;
        case '3':
            configureAPCredentials();
            break;
        case '4':
            configureClientCredentials();
            break;
        case '5':
            displayMenu();
            return;
    }

    wifiMenu();
}

void configureAPCredentials() {
    Serial.println("\n=== Configure AP Credentials ===");
    Serial.print("Enter SSID: ");
    apSSID = readLine();

    Serial.print("Enter Password (min 8 chars): ");
    apPassword = readLine();

    if (apPassword.length() >= 8) {
        saveConfig();
        Serial.println("AP credentials updated!");
    } else {
        Serial.println("Error: Password must be at least 8 characters!");
    }
}

void configureClientCredentials() {
    Serial.println("\n=== Configure Client Credentials ===");
    Serial.print("Enter SSID: ");
    clientSSID = readLine();

    Serial.print("Enter Password: ");
    clientPassword = readLine();

    saveConfig();
    Serial.println("Client credentials updated!");
}

void canMenu() {
    Serial.println("\n===== CAN Bus Configuration =====");
    Serial.println("1. Configure CAN1 Speed");
    Serial.println("2. Configure CAN2 Speed");
    Serial.println("3. Back to Main Menu");
    Serial.print("\nEnter choice: ");

    while (!Serial.available()) delay(10);
    char choice = Serial.read();
    while (Serial.available()) Serial.read();
    Serial.println(choice);

    switch (choice) {
        case '1':
            configureCANSpeed(1);
            break;
        case '2':
            configureCANSpeed(2);
            break;
        case '3':
            displayMenu();
            return;
    }

    canMenu();
}

void configureCANSpeed(uint8_t channel) {
    Serial.printf("\n=== Configure CAN%d Speed ===\n", channel);
    Serial.println("1. 125 kbps");
    Serial.println("2. 250 kbps");
    Serial.println("3. 500 kbps");
    Serial.println("4. 1000 kbps");
    Serial.print("\nEnter choice: ");

    while (!Serial.available()) delay(10);
    char choice = Serial.read();
    while (Serial.available()) Serial.read();
    Serial.println(choice);

    CANSpeed_t newSpeed = CAN_SPEED_500KBPS;
    switch (choice) {
        case '1': newSpeed = CAN_SPEED_125KBPS; break;
        case '2': newSpeed = CAN_SPEED_250KBPS; break;
        case '3': newSpeed = CAN_SPEED_500KBPS; break;
        case '4': newSpeed = CAN_SPEED_1000KBPS; break;
        default:
            Serial.println("Invalid choice!");
            return;
    }

    if (channel == 1) {
        can1Speed = newSpeed;
        stopCAN1();
        startCAN1();
    } else {
        can2Speed = newSpeed;
        can2Running = false;
        startCAN2();
    }

    saveConfig();
    Serial.printf("CAN%d speed updated!\n", channel);
}

void snifferMenu() {
    Serial.println("\n===== CAN Sniffer =====");

    if (sniffingEnabled) {
        Serial.println("Sniffer is RUNNING");
        Serial.printf("Monitoring: CAN%d\n", sniffingChannel + 1);
        Serial.println("\n1. Stop Sniffer");
        Serial.println("2. Back to Main Menu");
    } else {
        Serial.println("Sniffer is STOPPED");
        Serial.println("\n1. Start Sniffer on CAN1");
        Serial.println("2. Start Sniffer on CAN2");
        Serial.println("3. Back to Main Menu");
    }

    Serial.print("\nEnter choice: ");

    while (!Serial.available()) delay(10);
    char choice = Serial.read();
    while (Serial.available()) Serial.read();
    Serial.println(choice);

    if (sniffingEnabled) {
        if (choice == '1') {
            sniffingEnabled = false;
            Serial.println("Sniffer stopped");
        }
    } else {
        if (choice == '1') {
            sniffingEnabled = true;
            sniffingChannel = 0;
            Serial.println("Starting sniffer on CAN1...");
        } else if (choice == '2') {
            sniffingEnabled = true;
            sniffingChannel = 1;
            Serial.println("Starting sniffer on CAN2...");
        }
    }

    if (choice == '2' || choice == '3') {
        displayMenu();
    } else {
        snifferMenu();
    }
}

void printStatus() {
    Serial.println("\n========== SYSTEM STATUS ==========");
    Serial.print("WiFi Mode: ");
    Serial.println((wifiMode == GATEWAY_WIFI_AP) ? "Access Point" : "Client");
    Serial.print("IP Address: ");
    Serial.println((wifiMode == GATEWAY_WIFI_AP) ? WiFi.softAPIP().toString() : WiFi.localIP().toString());
    Serial.println();
    Serial.print("CAN1: ");
    Serial.println(can1Running ? "Running" : "Stopped");
    Serial.print("CAN2: ");
    Serial.println(can2Running ? "Running" : "Stopped");
    Serial.println("===================================\n");
}

void resetConfig() {
    Serial.println("\nResetting all settings to defaults...");
    prefs.clear();
    Serial.println("Reset complete! Rebooting...");
    delay(1000);
    ESP.restart();
}

String readLine() {
    String input = "";
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
                if (input.length() > 0) {
                    Serial.println();
                    return input;
                }
            } else if (c == 8 || c == 127) { // Backspace
                if (input.length() > 0) {
                    input.remove(input.length() - 1);
                    Serial.print("\b \b");
                }
            } else {
                input += c;
                Serial.print(c);
            }
        }
        delay(10);
    }
}

// ============================================
// PSRAM & DUAL-CORE FUNCTIONS
// ============================================

void initPSRAM() {
    if (psramFound()) {
        psramAvailable = true;
        psramTotal = ESP.getPsramSize();
        Serial.println("PSRAM: Detected");
        Serial.printf("PSRAM Total: %d bytes (%.2f MB)\n", psramTotal, psramTotal / 1048576.0);
    } else {
        psramAvailable = false;
        Serial.println("PSRAM: Not available");
    }
}

void updateCoreStats() {
    // Update core load estimation
    static uint32_t lastCheck = 0;
    uint32_t now = millis();

    if (now - lastCheck > 1000) {
        // Only check if task handles are valid
        if (networkTaskHandle != NULL && canTaskHandle != NULL) {
            // Estimate load based on task stack high water marks
            // Lower high water mark = more stack usage = higher load
            UBaseType_t core0Stack = uxTaskGetStackHighWaterMark(networkTaskHandle);
            UBaseType_t core1Stack = uxTaskGetStackHighWaterMark(canTaskHandle);

            // Convert to percentage (inverse - more used = higher load)
            // This is a rough estimate, not precise CPU utilization
            core0Load = ((16384 - core0Stack) * 100.0) / 16384.0;
            core1Load = ((16384 - core1Stack) * 100.0) / 16384.0;

            // Clamp values
            if (core0Load < 0) core0Load = 0;
            if (core0Load > 100) core0Load = 100;
            if (core1Load < 0) core1Load = 0;
            if (core1Load > 100) core1Load = 100;
        }

        lastCheck = now;
    }

    // Update PSRAM usage
    if (psramAvailable) {
        psramUsed = psramTotal - ESP.getFreePsram();
    }
}

// ============================================
// DUAL-CORE TASKS
// ============================================

// Task for CAN bus processing (Core 1 - Application CPU)
void canProcessingTask(void* parameter) {
    CANMessage msg;
    uint32_t id;
    uint8_t data[8];
    uint8_t len;

    Serial.println("[Core 1] CAN processing task started");

    while (true) {
        // Process CAN1 (TWAI)
        if (receiveCAN1(id, data, len)) {
            can1MessageCount++;

            // Prepare message for queues
            msg.channel = 1;
            msg.id = id;
            msg.len = len;
            msg.timestamp = millis();
            memcpy(msg.data, data, len);

            // Send to web queue if monitoring enabled
            if (webMonitoringEnabled && canToWebQueue != NULL) {
                xQueueSend(canToWebQueue, &msg, 0);
            }

            // Send to serial queue if sniffing enabled
            if (sniffingEnabled && sniffingChannel == 0 && serialOutputQueue != NULL) {
                xQueueSend(serialOutputQueue, &msg, 0);
            }
        }

        // Process CAN2 (MCP2515)
        if (receiveCAN2(id, data, len)) {
            can2MessageCount++;

            // Prepare message for queues
            msg.channel = 2;
            msg.id = id;
            msg.len = len;
            msg.timestamp = millis();
            memcpy(msg.data, data, len);

            // Send to web queue if monitoring enabled
            if (webMonitoringEnabled && canToWebQueue != NULL) {
                xQueueSend(canToWebQueue, &msg, 0);
            }

            // Send to serial queue if sniffing enabled
            if (sniffingEnabled && sniffingChannel == 1 && serialOutputQueue != NULL) {
                xQueueSend(serialOutputQueue, &msg, 0);
            }
        }

        // Small delay to prevent CPU hogging
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// Task for network and WebSocket handling (Core 0 - Protocol CPU)
void networkTask(void* parameter) {
    CANMessage msg;

    Serial.println("[Core 0] Network task started");

    while (true) {
        // Process messages from CAN queue to WebSocket
        while (canToWebQueue != NULL && xQueueReceive(canToWebQueue, &msg, 0) == pdTRUE) {
            if (ws.count() > 0) {
                sendCANMessageToWeb(msg.channel, msg.id, msg.data, msg.len);
            }
        }

        // Process messages for serial output
        while (serialOutputQueue != NULL && xQueueReceive(serialOutputQueue, &msg, 0) == pdTRUE) {
            Serial.printf("[%lu] [CAN%d] %08X [%d] ",
                msg.timestamp, msg.channel, msg.id, msg.len);
            for (int i = 0; i < msg.len; i++) {
                Serial.printf("%02X ", msg.data[i]);
            }
            Serial.println();
        }

        // Cleanup WebSocket connections
        ws.cleanupClients();

        // Update statistics
        updateCoreStats();

        // Longer delay for network task
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ============================================
// SETUP
// ============================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.println("\n\n========================================");
    Serial.println("     CANIMEX GATEWAY v2.0");
    Serial.println("     ESP32-CAN-X2 Dual CAN Bus");
    Serial.println("     Dual-Core + PSRAM Edition");
    Serial.println("========================================\n");

    // Initialize PSRAM
    Serial.println("Checking PSRAM...");
    initPSRAM();

    // Create FreeRTOS queues
    Serial.println("Creating FreeRTOS queues...");
    if (psramAvailable) {
        // Use PSRAM for large queues
        canToWebQueue = xQueueCreate(100, sizeof(CANMessage));
        serialOutputQueue = xQueueCreate(50, sizeof(CANMessage));
    } else {
        // Smaller queues for regular RAM
        canToWebQueue = xQueueCreate(20, sizeof(CANMessage));
        serialOutputQueue = xQueueCreate(10, sizeof(CANMessage));
    }

    // Create mutex for config access
    configMutex = xSemaphoreCreateMutex();

    if (canToWebQueue != NULL && serialOutputQueue != NULL && configMutex != NULL) {
        Serial.printf("Queue sizes: Web=%d, Serial=%d\n",
            psramAvailable ? 100 : 20,
            psramAvailable ? 50 : 10);
    } else {
        Serial.println("ERROR: Failed to create queues/mutex!");
        Serial.println("Restarting...");
        delay(2000);
        ESP.restart();
    }

    // Load configuration
    loadConfig();

    // Initialize WiFi
    Serial.println("Initializing WiFi...");
    if (wifiMode == GATEWAY_WIFI_AP) {
        startWiFiAP();
    } else {
        startWiFiClient();
    }

    // Initialize CAN buses
    Serial.println("Initializing CAN buses...");
    startCAN1();
    startCAN2();

    // Start dual-core tasks BEFORE web server
    Serial.println("\nStarting dual-core tasks...");

    // CAN processing task on Core 1 (Application CPU)
    BaseType_t taskResult1 = xTaskCreatePinnedToCore(
        canProcessingTask,   // Task function
        "CANTask",          // Task name
        16384,              // Stack size (bytes) - increased for stability
        NULL,               // Parameters
        2,                  // Priority (higher = more important)
        &canTaskHandle,     // Task handle
        1                   // Core 1
    );

    // Network task on Core 0 (Protocol CPU)
    BaseType_t taskResult2 = xTaskCreatePinnedToCore(
        networkTask,        // Task function
        "NetworkTask",      // Task name
        16384,              // Stack size (bytes) - increased for stability
        NULL,               // Parameters
        1,                  // Priority
        &networkTaskHandle, // Task handle
        0                   // Core 0
    );

    if (taskResult1 == pdPASS && taskResult2 == pdPASS) {
        Serial.println("Core 0: Network & WebSocket");
        Serial.println("Core 1: CAN Processing");

        // Give tasks time to start and initialize
        delay(1000);

        // Initialize web server on Core 1 (where WiFi events run)
        Serial.println("\nInitializing Web Server...");
        setupWebServer();

        Serial.println("\n========================================");
        Serial.println("    System Ready!");
        Serial.println("========================================\n");

        printStatus();
        displayMenu();
    } else {
        Serial.println("ERROR: Failed to create tasks!");
        Serial.println("System cannot continue without tasks.");
        delay(2000);
        ESP.restart();
    }
}

// ============================================
// LOOP
// ============================================

void loop() {
    // Handle menu input (tasks handle CAN processing and networking)
    handleMenu();

    // Heartbeat LED
    if (millis() - lastHeartbeat >= 1000) {
        lastHeartbeat = millis();
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }

    delay(10);
}
