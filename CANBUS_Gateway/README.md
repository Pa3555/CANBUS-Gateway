# CANIMEX Gateway - Arduino IDE Project

ESP32-based CAN bus gateway for **ESP32-CAN-X2** from Autosport Labs.

## Hardware

- **Board**: ESP32-CAN-X2 from Autosport Labs
- **MCU**: ESP32-S3-WROOM-1-N8R8
- **CAN1**: Built-in TWAI controller (GPIO6/GPIO7)
- **CAN2**: MCP2515 external controller via SPI

## Features

- ✅ **Dual CAN Bus**: Two independent CAN channels with configurable speeds (125, 250, 500, 1000 kbps)
- ✅ **WiFi Modes**: Access Point (default) or Client mode
- ✅ **Web Interface**: Modern responsive web dashboard for configuration and monitoring
- ✅ **Real-time Monitoring**: WebSocket-based live CAN message streaming
- ✅ **CAN Bus Sniffing**: Serial and web-based message monitoring
- ✅ **CANopen Support**: Ready for joystick integration (SDO/PDO)
- ✅ **Serial Menu**: Easy configuration via Serial Monitor
- ✅ **REST API**: Full API for remote configuration
- ✅ **Persistent Settings**: All configurations saved to flash

## Default WiFi Settings

- **Mode**: Access Point (AP)
- **SSID**: `CANIMEX_GATEWAY`
- **Password**: `Canimex2026`
- **IP Address**: 192.168.4.1

## Arduino IDE Setup

### 1. Install ESP32 Board Support

1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add this URL to **Additional Board Manager URLs**:
   ```
   https://dl.espressif.com/dl/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for "esp32"
6. Install **"esp32 by Espressif Systems"**

### 2. Select Board

1. Go to **Tools → Board → esp32**
2. Select **"AutosportLabs ESP32-CAN-X2"**

### 3. Install Required Libraries

Go to **Tools → Manage Libraries** and install:

- **mcp_canbus** by Longan Labs (for MCP2515 controller)
- **ESPAsyncWebServer** by me-no-dev (for web interface)
- **AsyncTCP** by me-no-dev (dependency for ESPAsyncWebServer)
- **ArduinoJson** by Benoit Blanchon (for JSON API)

### 4. Upload

1. Connect your ESP32-CAN-X2 via USB
2. Select the correct Port in **Tools → Port**
3. Click **Upload**

## Pin Configuration

### CAN1 (Built-in TWAI Controller)
- **TX**: GPIO7
- **RX**: GPIO6

### CAN2 (MCP2515 Controller)
- **CS**: GPIO10
- **MOSI**: GPIO11
- **CLK**: GPIO12
- **MISO**: GPIO13
- **IRQ**: GPIO3

### Other
- **LED**: GPIO2

## Usage

### Serial Monitor

1. Open Serial Monitor at **115200 baud**
2. Use the menu to configure:
   - WiFi mode (AP/Client)
   - CAN bus speeds
   - Start/stop CAN sniffer

### Menu Options

```
========== MAIN MENU ==========
1. WiFi Configuration
2. CAN Bus Configuration
3. CAN Sniffer
4. View Status
5. Reset to Defaults
h. Show this menu
===============================
```

### 🌐 Web Interface

The gateway includes a modern, responsive web interface accessible from any browser!

**Access the Web Interface:**

1. **Connect to WiFi**:
   - **AP Mode** (default): Connect to `CANIMEX_GATEWAY` WiFi network
   - **Client Mode**: Gateway connects to your existing WiFi

2. **Open Browser**: Navigate to the IP address shown in Serial Monitor
   - **AP Mode**: `http://192.168.4.1`
   - **Client Mode**: Check Serial Monitor for assigned IP

3. **Web Dashboard Features**:
   - 📊 **System Status**: Real-time system information and uptime
   - 📡 **WiFi Configuration**: Change between AP/Client modes
   - 🔧 **CAN Configuration**: Set CAN bus speeds for both channels
   - 📡 **Live CAN Monitor**: Real-time CAN message streaming via WebSocket
   - 📈 **Message Statistics**: Track message counts for each channel
   - 🔄 **Reset Controls**: Reset to factory defaults

**Screenshot:**
```
┌────────────────────────────────────────┐
│   🚗 CANIMEX Gateway                   │
│   ESP32-CAN-X2 Dual CAN Bus Interface  │
│   WiFi: AP | CAN1: Running | CAN2: ✓ │
├────────────────────────────────────────┤
│ 📊 System Status  │ 📡 WiFi Config    │
│ IP: 192.168.4.1   │ Mode: [AP ▼]      │
│ CAN1: 500 kbps    │ SSID: [_______]   │
│ CAN2: 500 kbps    │ Pass: [_______]   │
│ Uptime: 1h 23m    │ [Save Config]     │
├────────────────────────────────────────┤
│ 📡 CAN Bus Monitor [✓ Real-time] [Clear]│
│ ┌────────────────────────────────────┐ │
│ │ [10:23:45] [CAN1] ID:0x123 [8]    │ │
│ │ 01 02 03 04 05 06 07 08           │ │
│ │ [10:23:46] [CAN2] ID:0x456 [4]    │ │
│ │ AA BB CC DD                       │ │
│ └────────────────────────────────────┘ │
│  CAN1: 1,234 msgs  │  CAN2: 567 msgs  │
└────────────────────────────────────────┘
```

**API Endpoints:**
- `GET /api/status` - Get system status (JSON)
- `POST /api/wifi` - Configure WiFi settings
- `POST /api/can` - Configure CAN settings
- `POST /api/monitor` - Toggle real-time monitoring
- `POST /api/reset` - Reset to defaults
- `WS /ws` - WebSocket for real-time CAN data

### WiFi Configuration

**Switch to AP Mode:**
1. Press `1` (WiFi Configuration)
2. Press `1` (Switch to AP Mode)
3. Device will restart as Access Point

**Switch to Client Mode:**
1. Press `1` (WiFi Configuration)
2. Press `4` (Configure Client Credentials)
3. Enter your WiFi SSID and password
4. Press `1`, then `2` (Switch to Client Mode)
5. Device will restart and connect to your WiFi

### CAN Bus Configuration

**Change CAN Speed:**
1. Press `2` (CAN Bus Configuration)
2. Select `1` for CAN1 or `2` for CAN2
3. Choose desired speed (125/250/500/1000 kbps)

### CAN Sniffer

**Start Sniffing:**
1. Press `3` (CAN Sniffer)
2. Press `1` for CAN1 or `2` for CAN2
3. CAN messages will stream to Serial Monitor

**Message Format:**
```
[timestamp] [CANx] ID [DLC] DATA
[12345] [CAN1] 00000123 [8] 01 02 03 04 05 06 07 08
```

## Wiring CAN Devices

### CAN Bus Connection

Connect your CAN devices to the terminal blocks on the ESP32-CAN-X2:

- **CAN1**: X1 connector (pins 1-4 on SV1 header)
  - Pin 1: CAN1_H
  - Pin 2: CAN1_L

- **CAN2**: X2 connector (pins 3-4 on SV1 header)
  - Pin 3: CAN2_H
  - Pin 4: CAN2_L

### Termination

- Add 120Ω resistors between CAN_H and CAN_L at both ends of the bus
- Maximum bus length depends on baud rate:
  - 1000 kbps: 40m max
  - 500 kbps: 100m max
  - 250 kbps: 250m max
  - 125 kbps: 500m max

## Troubleshooting

### CAN2 Not Working

If CAN2 (MCP2515) fails to initialize:
1. Check SPI wiring (shouldn't be needed, it's internal)
2. Verify MCP2515 clock speed matches (8MHz default)
3. Check Serial Monitor for error messages

### WiFi Issues

**Can't see AP:**
- Wait 30 seconds after boot
- Check if device restarted properly
- Try pressing `5` to reset to defaults

**Client mode won't connect:**
- Verify SSID and password are correct
- Check WiFi signal strength
- Device will fall back to AP mode if connection fails

### CAN Bus Issues

**No messages appearing:**
- Verify baud rate matches all devices on bus
- Check termination resistors (120Ω)
- Verify wiring: CAN_H, CAN_L, GND

**Bus-off errors:**
- Check for proper termination
- Reduce baud rate
- Check cable quality and length

## Advanced Features

### CANopen Joystick Support

The firmware includes CANopen protocol support for joystick integration. To add joystick functionality:

1. Connect CANopen joystick to CAN1 or CAN2
2. Note the joystick's Node ID (usually 1-127)
3. Modify code to read TPDO messages from the joystick
4. Parse axis and button data according to joystick specifications

Example message format (TPDO1 from Node ID 5):
```
CAN ID: 0x185 (0x180 + Node ID)
Data[0-1]: X-axis (signed 16-bit)
Data[2-3]: Y-axis (signed 16-bit)
Data[4-5]: Z-axis (signed 16-bit)
Data[6-7]: Button states
```

## Resources

- **Official Wiki**: https://wiki.autosportlabs.com/ESP32-CAN-X2
- **GitHub Examples**: https://github.com/autosportlabs/ESP32-CAN-X2
- **Product Page**: https://www.autosportlabs.com/product/esp32-can-x2-dual-can-bus-automotive-grade-development-board/

## License

MIT License - See LICENSE file for details

## Support

For issues or questions:
- Check the troubleshooting section above
- Visit the Autosport Labs forum
- Review the official documentation

## Changelog

### v1.0 - Initial Release
- Dual CAN bus support (TWAI + MCP2515)
- WiFi AP/Client modes with serial configuration
- CAN bus sniffing on both channels
- Persistent configuration storage
- Interactive serial menu system
