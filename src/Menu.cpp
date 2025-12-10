#include "Menu.h"

Menu::Menu(WiFiConfig* wifi, CANConfig* canCfg, CANopen* co0, CANopen* co1)
    : wifiConfig(wifi), canConfig(canCfg), canopen0(co0), canopen1(co1) {
}

void Menu::begin() {
    Serial.println("\n========================================");
    Serial.println("   CANIMEX GATEWAY - Menu System");
    Serial.println("========================================\n");
    displayMainMenu();
}

void Menu::process() {
    if (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            // Ignore
        } else {
            handleMainMenu(c);
        }
    }
}

void Menu::displayMainMenu() {
    Serial.println("\n========== MAIN MENU ==========");
    Serial.println("1. WiFi Configuration");
    Serial.println("2. CAN Bus Configuration");
    Serial.println("3. CAN Sniffer");
    Serial.println("4. CANopen Joystick");
    Serial.println("5. View Status");
    Serial.println("6. Reset to Defaults");
    Serial.println("h. Show this menu");
    Serial.println("===============================");
    Serial.print("\nEnter choice: ");
}

void Menu::handleMainMenu(char choice) {
    Serial.println(choice);

    switch (choice) {
        case '1':
            handleWiFiMenu();
            break;
        case '2':
            handleCANMenu();
            break;
        case '3':
            handleSnifferMenu();
            break;
        case '4':
            handleJoystickMenu();
            break;
        case '5':
            handleStatusMenu();
            break;
        case '6':
            Serial.println("\nResetting all settings to defaults...");
            wifiConfig->resetToDefaults();
            canConfig->resetToDefaults();
            Serial.println("Reset complete! Rebooting...");
            delay(1000);
            ESP.restart();
            break;
        case 'h':
        case 'H':
            displayMainMenu();
            break;
        default:
            Serial.println("\nInvalid choice!");
            displayMainMenu();
            break;
    }
}

void Menu::handleWiFiMenu() {
    Serial.println("\n===== WiFi Configuration =====");
    Serial.println("1. Select WiFi Mode (AP/Client)");
    Serial.println("2. Configure AP Credentials");
    Serial.println("3. Configure Client Credentials");
    Serial.println("4. Back to Main Menu");
    Serial.print("\nEnter choice: ");

    while (!Serial.available()) delay(10);
    char choice = Serial.read();
    while (Serial.available()) Serial.read(); // Clear buffer
    Serial.println(choice);

    switch (choice) {
        case '1':
            configureWiFiMode();
            break;
        case '2':
            configureAPCredentials();
            break;
        case '3':
            configureClientCredentials();
            break;
        case '4':
            displayMainMenu();
            return;
        default:
            Serial.println("Invalid choice!");
            break;
    }

    handleWiFiMenu();
}

void Menu::configureWiFiMode() {
    Serial.println("\n=== Select WiFi Mode ===");
    Serial.println("1. Access Point (AP)");
    Serial.println("2. Client (Station)");
    Serial.print("\nEnter choice: ");

    while (!Serial.available()) delay(10);
    char choice = Serial.read();
    while (Serial.available()) Serial.read();
    Serial.println(choice);

    if (choice == '1') {
        wifiConfig->setMode(WIFI_MODE_AP);
        Serial.println("WiFi mode set to Access Point");
    } else if (choice == '2') {
        wifiConfig->setMode(WIFI_MODE_CLIENT);
        Serial.println("WiFi mode set to Client");
    } else {
        Serial.println("Invalid choice!");
    }
}

void Menu::configureAPCredentials() {
    Serial.println("\n=== Configure AP Credentials ===");
    Serial.print("Enter SSID: ");
    String ssid = readLine();

    Serial.print("Enter Password (min 8 chars): ");
    String password = readLine();

    if (password.length() >= 8) {
        wifiConfig->setAPCredentials(ssid.c_str(), password.c_str());
        Serial.println("AP credentials updated!");
    } else {
        Serial.println("Error: Password must be at least 8 characters!");
    }
}

void Menu::configureClientCredentials() {
    Serial.println("\n=== Configure Client Credentials ===");
    Serial.print("Enter SSID: ");
    String ssid = readLine();

    Serial.print("Enter Password: ");
    String password = readLine();

    wifiConfig->setClientCredentials(ssid.c_str(), password.c_str());
    Serial.println("Client credentials updated!");
}

void Menu::handleCANMenu() {
    Serial.println("\n===== CAN Bus Configuration =====");
    Serial.println("1. Configure CAN0 Speed");
    Serial.println("2. Configure CAN1 Speed");
    Serial.println("3. Back to Main Menu");
    Serial.print("\nEnter choice: ");

    while (!Serial.available()) delay(10);
    char choice = Serial.read();
    while (Serial.available()) Serial.read();
    Serial.println(choice);

    switch (choice) {
        case '1':
            configureCANChannel(0);
            break;
        case '2':
            configureCANChannel(1);
            break;
        case '3':
            displayMainMenu();
            return;
        default:
            Serial.println("Invalid choice!");
            break;
    }

    handleCANMenu();
}

void Menu::configureCANChannel(uint8_t channel) {
    Serial.printf("\n=== Configure CAN%d Speed ===\n", channel);
    CANSpeed speed = selectCANSpeed();
    canConfig->setSpeed(channel, speed);
    Serial.printf("CAN%d speed set to %d kbps\n", channel, speed);
}

CANSpeed Menu::selectCANSpeed() {
    Serial.println("\nSelect CAN Speed:");
    Serial.println("1. 10 kbps");
    Serial.println("2. 20 kbps");
    Serial.println("3. 50 kbps");
    Serial.println("4. 100 kbps");
    Serial.println("5. 125 kbps");
    Serial.println("6. 250 kbps");
    Serial.println("7. 500 kbps");
    Serial.println("8. 1000 kbps");
    Serial.print("\nEnter choice: ");

    while (!Serial.available()) delay(10);
    char choice = Serial.read();
    while (Serial.available()) Serial.read();
    Serial.println(choice);

    switch (choice) {
        case '1': return CAN_SPEED_10KBPS;
        case '2': return CAN_SPEED_20KBPS;
        case '3': return CAN_SPEED_50KBPS;
        case '4': return CAN_SPEED_100KBPS;
        case '5': return CAN_SPEED_125KBPS;
        case '6': return CAN_SPEED_250KBPS;
        case '7': return CAN_SPEED_500KBPS;
        case '8': return CAN_SPEED_1000KBPS;
        default:
            Serial.println("Invalid choice! Using 500 kbps");
            return CAN_SPEED_500KBPS;
    }
}

void Menu::handleSnifferMenu() {
    Serial.println("\n===== CAN Sniffer =====");

    if (canConfig->isSniffing()) {
        Serial.println("Sniffer is currently RUNNING");
        Serial.printf("Monitoring: CAN%d\n", canConfig->getSniffingChannel());
        Serial.println("\n1. Stop Sniffer");
        Serial.println("2. Back to Main Menu");
    } else {
        Serial.println("Sniffer is currently STOPPED");
        Serial.println("\n1. Start Sniffer on CAN0");
        Serial.println("2. Start Sniffer on CAN1");
        Serial.println("3. Back to Main Menu");
    }

    Serial.print("\nEnter choice: ");

    while (!Serial.available()) delay(10);
    char choice = Serial.read();
    while (Serial.available()) Serial.read();
    Serial.println(choice);

    if (canConfig->isSniffing()) {
        if (choice == '1') {
            canConfig->stopSniffing();
        }
    } else {
        if (choice == '1') {
            canConfig->startSniffing(0);
        } else if (choice == '2') {
            canConfig->startSniffing(1);
        }
    }

    if (choice != '2' && choice != '3') {
        handleSnifferMenu();
    } else {
        displayMainMenu();
    }
}

void Menu::handleJoystickMenu() {
    Serial.println("\n===== CANopen Joystick =====");
    Serial.println("1. Configure Joystick (CAN0)");
    Serial.println("2. Configure Joystick (CAN1)");
    Serial.println("3. Test Joystick (CAN0)");
    Serial.println("4. Test Joystick (CAN1)");
    Serial.println("5. Back to Main Menu");
    Serial.print("\nEnter choice: ");

    while (!Serial.available()) delay(10);
    char choice = Serial.read();
    while (Serial.available()) Serial.read();
    Serial.println(choice);

    switch (choice) {
        case '1':
            Serial.println("Configuring joystick on CAN0...");
            configureJoystick();
            break;
        case '2':
            Serial.println("Configuring joystick on CAN1...");
            configureJoystick();
            break;
        case '3':
            Serial.println("Testing joystick on CAN0 (Press any key to stop)...");
            testJoystick();
            break;
        case '4':
            Serial.println("Testing joystick on CAN1 (Press any key to stop)...");
            testJoystick();
            break;
        case '5':
            displayMainMenu();
            return;
        default:
            Serial.println("Invalid choice!");
            break;
    }

    handleJoystickMenu();
}

void Menu::configureJoystick() {
    Serial.print("\nEnter CANopen Node ID (1-127): ");
    int nodeId = readInt();

    if (nodeId >= 1 && nodeId <= 127) {
        canopen0->enableJoystickPDO(nodeId);
        Serial.printf("Joystick configured for Node ID %d\n", nodeId);
    } else {
        Serial.println("Invalid Node ID!");
    }
}

void Menu::testJoystick() {
    Serial.print("Enter CANopen Node ID (1-127): ");
    int nodeId = readInt();

    if (nodeId < 1 || nodeId > 127) {
        Serial.println("Invalid Node ID!");
        return;
    }

    Serial.println("\nReading joystick data... (Press any key to stop)\n");

    while (!Serial.available()) {
        JoystickData joystick;
        if (canopen0->readJoystick(nodeId, joystick)) {
            canopen0->printJoystickData(joystick);
        }
        delay(50);
    }

    while (Serial.available()) Serial.read(); // Clear buffer
}

void Menu::handleStatusMenu() {
    Serial.println("\n========== SYSTEM STATUS ==========");
    wifiConfig->printStatus();
    canConfig->printStatus();
    Serial.println("===================================\n");
    displayMainMenu();
}

String Menu::readLine() {
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

int Menu::readInt() {
    String input = readLine();
    return input.toInt();
}

void Menu::clearScreen() {
    Serial.print("\033[2J\033[H");
}

void Menu::waitForKey() {
    Serial.println("\nPress any key to continue...");
    while (!Serial.available()) delay(10);
    while (Serial.available()) Serial.read();
}
