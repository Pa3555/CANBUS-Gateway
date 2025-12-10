#include "WiFiConfig.h"

WiFiConfig::WiFiConfig() {
    currentMode = WIFI_MODE_AP;
    apSSID = DEFAULT_AP_SSID;
    apPassword = DEFAULT_AP_PASSWORD;
    clientSSID = "";
    clientPassword = "";
}

void WiFiConfig::begin() {
    preferences.begin("wifi-config", false);
    loadConfig();
    applyWiFiMode();
}

void WiFiConfig::loadConfig() {
    // Load WiFi mode
    currentMode = (WiFiMode)preferences.getUChar("mode", WIFI_MODE_AP);

    // Load AP credentials
    apSSID = preferences.getString("ap_ssid", DEFAULT_AP_SSID);
    apPassword = preferences.getString("ap_pass", DEFAULT_AP_PASSWORD);

    // Load Client credentials
    clientSSID = preferences.getString("cli_ssid", "");
    clientPassword = preferences.getString("cli_pass", "");

    Serial.println("Configuration loaded from flash");
}

void WiFiConfig::saveConfig() {
    preferences.putUChar("mode", currentMode);
    preferences.putString("ap_ssid", apSSID);
    preferences.putString("ap_pass", apPassword);
    preferences.putString("cli_ssid", clientSSID);
    preferences.putString("cli_pass", clientPassword);

    Serial.println("Configuration saved to flash");
}

void WiFiConfig::resetToDefaults() {
    preferences.clear();
    currentMode = WIFI_MODE_AP;
    apSSID = DEFAULT_AP_SSID;
    apPassword = DEFAULT_AP_PASSWORD;
    clientSSID = "";
    clientPassword = "";
    saveConfig();
    Serial.println("Configuration reset to defaults");
}

void WiFiConfig::setMode(WiFiMode mode) {
    if (currentMode != mode) {
        currentMode = mode;
        disconnect();
        applyWiFiMode();
        saveConfig();
    }
}

WiFiMode WiFiConfig::getMode() {
    return currentMode;
}

void WiFiConfig::startAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSSID.c_str(), apPassword.c_str(), DEFAULT_AP_CHANNEL,
                DEFAULT_AP_HIDDEN, DEFAULT_AP_MAX_CONNECTIONS);

    Serial.println("\n=== Access Point Started ===");
    Serial.print("SSID: ");
    Serial.println(apSSID);
    Serial.print("Password: ");
    Serial.println(apPassword);
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("===========================\n");
}

void WiFiConfig::startClient() {
    if (clientSSID.length() == 0) {
        Serial.println("ERROR: Client SSID not configured!");
        Serial.println("Falling back to AP mode...");
        currentMode = WIFI_MODE_AP;
        startAP();
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
        Serial.println("============================\n");
    } else {
        Serial.println("\nERROR: Failed to connect to WiFi!");
        Serial.println("Falling back to AP mode...");
        currentMode = WIFI_MODE_AP;
        startAP();
    }
}

void WiFiConfig::disconnect() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(100);
}

void WiFiConfig::setAPCredentials(const char* ssid, const char* password) {
    apSSID = ssid;
    apPassword = password;
    saveConfig();

    if (currentMode == WIFI_MODE_AP) {
        disconnect();
        startAP();
    }
}

String WiFiConfig::getAPSSID() {
    return apSSID;
}

String WiFiConfig::getAPPassword() {
    return apPassword;
}

void WiFiConfig::setClientCredentials(const char* ssid, const char* password) {
    clientSSID = ssid;
    clientPassword = password;
    saveConfig();
}

String WiFiConfig::getClientSSID() {
    return clientSSID;
}

String WiFiConfig::getClientPassword() {
    return clientPassword;
}

bool WiFiConfig::isClientConnected() {
    return (currentMode == WIFI_MODE_CLIENT && WiFi.status() == WL_CONNECTED);
}

String WiFiConfig::getIPAddress() {
    if (currentMode == WIFI_MODE_AP) {
        return WiFi.softAPIP().toString();
    } else {
        return WiFi.localIP().toString();
    }
}

String WiFiConfig::getStatusString() {
    String status = "WiFi Mode: ";
    status += (currentMode == WIFI_MODE_AP) ? "Access Point" : "Client";
    status += "\nIP: " + getIPAddress();

    if (currentMode == WIFI_MODE_CLIENT) {
        status += "\nConnected: ";
        status += (WiFi.status() == WL_CONNECTED) ? "Yes" : "No";
    }

    return status;
}

void WiFiConfig::printStatus() {
    Serial.println("\n=== WiFi Status ===");
    Serial.println(getStatusString());
    Serial.println("===================\n");
}

void WiFiConfig::applyWiFiMode() {
    if (currentMode == WIFI_MODE_AP) {
        startAP();
    } else {
        startClient();
    }
}
