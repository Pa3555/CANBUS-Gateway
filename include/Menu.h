#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include "WiFiConfig.h"
#include "CANConfig.h"
#include "CANopen.h"

class Menu {
public:
    Menu(WiFiConfig* wifi, CANConfig* canCfg, CANopen* canopen0, CANopen* canopen1);

    void begin();
    void process();
    void displayMainMenu();

private:
    WiFiConfig* wifiConfig;
    CANConfig* canConfig;
    CANopen* canopen0;
    CANopen* canopen1;

    String inputBuffer;

    // Menu handlers
    void handleMainMenu(char choice);
    void handleWiFiMenu();
    void handleCANMenu();
    void handleSnifferMenu();
    void handleJoystickMenu();
    void handleStatusMenu();

    // WiFi menu functions
    void configureWiFiMode();
    void configureAPCredentials();
    void configureClientCredentials();

    // CAN menu functions
    void configureCANSpeed();
    void configureCANChannel(uint8_t channel);

    // Sniffer functions
    void startSniffer();
    void stopSniffer();

    // Joystick functions
    void configureJoystick();
    void testJoystick();

    // Helper functions
    String readLine();
    int readInt();
    CANSpeed selectCANSpeed();
    void clearScreen();
    void waitForKey();
};

#endif // MENU_H
