#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

// WiFi modes
enum WiFiMode {
    WIFI_MODE_AP = 0,
    WIFI_MODE_CLIENT = 1
};

// Default WiFi credentials
#define DEFAULT_AP_SSID "CANIMEX_GATEWAY"
#define DEFAULT_AP_PASSWORD "Canimex2026"
#define DEFAULT_AP_CHANNEL 1
#define DEFAULT_AP_HIDDEN false
#define DEFAULT_AP_MAX_CONNECTIONS 4

class WiFiConfig {
public:
    WiFiConfig();
    void begin();
    void loadConfig();
    void saveConfig();
    void resetToDefaults();

    // WiFi mode management
    void setMode(WiFiMode mode);
    WiFiMode getMode();
    void startAP();
    void startClient();
    void disconnect();

    // AP mode settings
    void setAPCredentials(const char* ssid, const char* password);
    String getAPSSID();
    String getAPPassword();

    // Client mode settings
    void setClientCredentials(const char* ssid, const char* password);
    String getClientSSID();
    String getClientPassword();
    bool isClientConnected();

    // Status
    String getIPAddress();
    String getStatusString();
    void printStatus();

private:
    Preferences preferences;
    WiFiMode currentMode;

    // AP mode credentials
    String apSSID;
    String apPassword;

    // Client mode credentials
    String clientSSID;
    String clientPassword;

    // Helper methods
    void applyWiFiMode();
};

#endif // WIFI_CONFIG_H
