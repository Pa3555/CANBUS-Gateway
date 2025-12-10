# Quick Start Guide

## What You Need
- ESP32 CAN X2 board from Autosport Labs
- USB cable
- CAN devices to connect (optional for initial setup)
- Computer with Arduino IDE or PlatformIO

## 5-Minute Setup

### Step 1: Install Software

**Option A: PlatformIO (Recommended)**
```bash
# Clone the repository
git clone <repository-url>
cd CANBUS-Gateway

# Upload using PlatformIO
pio run --target upload
```

**Option B: Arduino IDE**
1. Install Arduino IDE
2. Install ESP32 board support
3. Open `CANBUS_Gateway.ino`
4. Select board: "ESP32 Dev Module"
5. Click Upload

### Step 2: Connect

1. **Power on** the ESP32 CAN X2
2. **Connect** to WiFi:
   - SSID: `CANIMEX_GATEWAY`
   - Password: `Canimex2026`
3. **Open Serial Monitor** at 115200 baud

### Step 3: Test

You should see:
```
========================================
     CANIMEX GATEWAY v1.0
     ESP32 CAN X2 - Dual CAN Bus
========================================

========== MAIN MENU ==========
1. WiFi Configuration
2. CAN Bus Configuration
3. CAN Sniffer
4. CANopen Joystick
5. View Status
6. Reset to Defaults
h. Show this menu
===============================
```

## Common Tasks

### Change WiFi to Client Mode
1. Press `1` (WiFi Configuration)
2. Press `3` (Configure Client Credentials)
3. Enter your WiFi SSID and password
4. Press `1` again, then `1` (Select WiFi Mode)
5. Press `2` (Client mode)

### Start CAN Sniffer
1. Connect CAN device to CAN0 or CAN1
2. Press `3` (CAN Sniffer)
3. Press `1` (CAN0) or `2` (CAN1)
4. Watch CAN messages scroll by!

### Configure CAN Speed
1. Press `2` (CAN Bus Configuration)
2. Press `1` (CAN0) or `2` (CAN1)
3. Select speed (default is 500 kbps)

### Test CANopen Joystick
1. Connect joystick to CAN0 or CAN1
2. Make sure joystick baud rate matches gateway
3. Press `4` (CANopen Joystick)
4. Press `3` (Test CAN0) or `4` (Test CAN1)
5. Enter joystick Node ID
6. Move joystick and watch values!

## Troubleshooting

### Can't see WiFi AP
- Wait 30 seconds after power on
- Check if ESP32 LED is on
- Try power cycle

### No CAN messages
- Check termination resistors (120Ω)
- Verify baud rate matches all devices
- Check wiring: CAN_H, CAN_L, GND

### Serial monitor shows garbage
- Set baud rate to 115200
- Try different USB cable
- Check board selection in Arduino IDE

## Next Steps

- Read [USAGE_GUIDE.md](docs/USAGE_GUIDE.md) for detailed features
- See [HARDWARE_SETUP.md](docs/HARDWARE_SETUP.md) for wiring
- Check [CANOPEN_REFERENCE.md](docs/CANOPEN_REFERENCE.md) for CANopen details

## Need Help?

Open an issue on GitHub with:
- What you're trying to do
- What happens instead
- Serial monitor output
- Your hardware setup
