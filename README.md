# CANBUS Gateway - ESP32 CAN X2

ESP32-based CAN bus gateway with dual CAN channels, WiFi connectivity, and CANopen joystick support.

## Hardware
- **Device**: ESP32 CAN X2 from Autosport Labs
- **CAN Channels**: 2 independent CAN bus interfaces
- **Connectivity**: WiFi (AP/Client mode)

## Features
- **Dual CAN Bus**: Two independent CAN channels with configurable speeds
- **WiFi Modes**:
  - Access Point (AP) mode - Default
  - Client (Station) mode
- **CAN Bus Sniffing**: Monitor and log CAN messages
- **CANopen Support**: SDO/PDO for joystick devices
- **Configuration Menu**: Serial interface for settings
- **Persistent Settings**: Configuration saved to flash memory

## Default WiFi Settings
- **Mode**: Access Point (AP)
- **SSID**: CANIMEX_GATEWAY
- **Password**: Canimex2026
- **IP Address**: 192.168.4.1

## Getting Started

### Prerequisites
- Arduino IDE 2.x or PlatformIO
- ESP32 board support
- Required libraries (see platformio.ini or Arduino Libraries section)

### Required Libraries
- ESP32CAN
- Preferences (built-in)
- WiFi (built-in)
- WebServer (built-in)

### Installation
1. Clone this repository
2. Open in Arduino IDE or PlatformIO
3. Upload to ESP32 CAN X2 device

### Configuration
Connect via serial monitor (115200 baud) to access the configuration menu:
- `1` - Configure WiFi mode
- `2` - Configure CAN bus settings
- `3` - View current settings
- `4` - Reset to defaults

## CAN Bus Configuration
Both CAN channels support speeds: 10, 20, 50, 100, 125, 250, 500, 1000 kbps

## License
MIT License
