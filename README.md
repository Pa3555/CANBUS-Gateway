# CANBUS Gateway - ESP32-CAN-X2 v2.0

ESP32-based CAN bus gateway for **ESP32-CAN-X2** from Autosport Labs with dual CAN channels, WiFi connectivity, dual-core processing, and PSRAM support.

## Hardware
- **Board**: ESP32-CAN-X2 from Autosport Labs
- **MCU**: ESP32-S3-WROOM-1-N8R8
  - Dual-core Xtensa LX6 CPU @ 240 MHz
  - 384 KB ROM, 512 KB SRAM, 8 MB PSRAM
- **Power**: 5-40V (automotive/industrial grade)
- **CAN1**: Built-in TWAI controller (GPIO6/GPIO7)
- **CAN2**: MCP2515 external controller via SPI
- **Connectivity**: WiFi (AP/Client mode)

## Features

### Core Functionality
- ✅ **Dual CAN Bus**: Two independent CAN channels (125/250/500/1000 kbps)
- ✅ **WiFi Modes**: Access Point (default) or Client mode
- ✅ **Web Interface**: Modern responsive dashboard with real-time monitoring
- ✅ **WebSocket Streaming**: Live CAN message updates
- ✅ **CAN Bus Sniffing**: Real-time message monitoring (serial + web)
- ✅ **CANopen Support**: Ready for joystick integration
- ✅ **Serial Configuration**: Easy menu-driven setup
- ✅ **REST API**: Full JSON API for remote configuration
- ✅ **Persistent Settings**: All configs saved to flash

### Performance Optimizations (v2.0)
- 🚀 **Dual-Core Architecture**:
  - Core 0: Network & WebSocket handling
  - Core 1: Dedicated CAN bus processing
- 🚀 **PSRAM Support**: 8MB PSRAM for large message buffers
  - 100 message queue (with PSRAM) vs 20 (without)
  - Future-ready for data logging and advanced features
- 🚀 **FreeRTOS Tasks**: Non-blocking parallel processing
- 🚀 **Real-time Stats**: Core load and memory usage monitoring

## Default WiFi Settings
- **Mode**: Access Point (AP)
- **SSID**: `CANIMEX_GATEWAY`
- **Password**: `Canimex2026`
- **IP Address**: 192.168.4.1

## Quick Start (Arduino IDE)

### 1. Install ESP32 Board Support
Add to Arduino IDE Board Manager:
```
https://dl.espressif.com/dl/package_esp32_index.json
```

### 2. Install Libraries

**Via Library Manager:**
- **mcp_canbus** by Longan Labs
- **ArduinoJson** by Benoit Blanchon
- **AsyncTCP** by me-no-dev

**Manual Installation (ESP32 v3.x compatible):**
- **ESPAsyncWebServer** (mathieucarbou's ESP32 v3.x fork):
  1. Download: https://github.com/mathieucarbou/ESPAsyncWebServer/archive/refs/heads/master.zip
  2. Arduino IDE → Sketch → Include Library → Add .ZIP Library
  3. Select the downloaded ZIP file

> **Note**: The official ESPAsyncWebServer by me-no-dev is not compatible with ESP32 v3.x. Use mathieucarbou's fork instead.

### 3. Select Board and Enable PSRAM
- Tools → Board → **"ESP32S3 Dev Module"** or **"AutosportLabs ESP32-CAN-X2"**
- Tools → PSRAM → **"OPI PSRAM"** (enables 8MB PSRAM support)

### 4. Upload
Open `CANBUS_Gateway/CANBUS_Gateway.ino` and upload!

### 5. Access Web Interface
- Connect to WiFi `CANIMEX_GATEWAY` (password: `Canimex2026`)
- Open browser to **http://192.168.4.1**
- Enjoy the modern web dashboard! 🎉

## Pin Configuration

### CAN1 (Built-in TWAI)
- **TX**: GPIO7
- **RX**: GPIO6

### CAN2 (MCP2515)
- **CS**: GPIO10
- **MOSI**: GPIO11
- **CLK**: GPIO12
- **MISO**: GPIO13
- **IRQ**: GPIO3

### Other
- **LED**: GPIO2

## Usage

Connect via Serial Monitor (115200 baud):

```
========== MAIN MENU ==========
1. WiFi Configuration
2. CAN Bus Configuration
3. CAN Sniffer
4. View Status
5. Reset to Defaults
===============================
```

### Configuration Options
- **WiFi Mode**: Switch between AP and Client modes
- **CAN Speed**: Configure each channel independently (125/250/500/1000 kbps)
- **CAN Sniffer**: Monitor CAN traffic in real-time

## Documentation

- **[CANBUS_Gateway/README.md](CANBUS_Gateway/README.md)** - Complete Arduino IDE guide
- **[QUICKSTART.md](QUICKSTART.md)** - 5-minute setup guide
- **[docs/HARDWARE_SETUP.md](docs/HARDWARE_SETUP.md)** - Wiring and hardware info
- **[docs/USAGE_GUIDE.md](docs/USAGE_GUIDE.md)** - Detailed features guide
- **[docs/CANOPEN_REFERENCE.md](docs/CANOPEN_REFERENCE.md)** - CANopen technical details

## Troubleshooting

### Compilation Errors

**`mbedtls_md5_starts_ret` not declared / ESPAsyncWebServer errors**
- **Cause**: Old ESPAsyncWebServer library incompatible with ESP32 v3.x
- **Fix**: Install mathieucarbou's ESP32 v3.x compatible fork (see installation instructions above)
- Uninstall old "ESPAsyncWebServer by me-no-dev" first

**`StaticJsonDocument` deprecated warnings**
- **Status**: Warnings only, code will compile and work
- **Info**: ArduinoJson v7+ prefers `JsonDocument` but `StaticJsonDocument` still works
- Safe to ignore for now

**PSRAM not detected**
- **Check**: Tools → PSRAM → "OPI PSRAM" is enabled
- **Note**: Code works without PSRAM (uses smaller message buffers)

**Upload fails**
- Ensure correct port selected: Tools → Port
- Hold BOOT button during upload if needed
- Check USB cable supports data (not just power)

## Resources

- **Official Wiki**: https://wiki.autosportlabs.com/ESP32-CAN-X2
- **GitHub Examples**: https://github.com/autosportlabs/ESP32-CAN-X2
- **Product Page**: https://www.autosportlabs.com/product/esp32-can-x2-dual-can-bus-automotive-grade-development-board/

## License
MIT License
