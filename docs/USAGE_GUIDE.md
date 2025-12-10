# Usage Guide

## First Time Setup

1. **Power on the device**
   - Connect via USB or external power
   - Wait for boot sequence (about 5 seconds)

2. **Connect to Serial Monitor**
   - Baud rate: 115200
   - Line ending: Newline or CR+LF
   - You should see the main menu

3. **Default WiFi AP Mode**
   - SSID: `CANIMEX_GATEWAY`
   - Password: `Canimex2026`
   - IP Address: `192.168.4.1`

## Menu System

### Main Menu Options

```
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

## WiFi Configuration

### Access Point (AP) Mode
- Device creates its own WiFi network
- Other devices connect to it
- Default mode on first boot
- IP address: 192.168.4.1

**To configure AP mode:**
1. Press `1` (WiFi Configuration)
2. Press `2` (Configure AP Credentials)
3. Enter new SSID
4. Enter new password (min 8 characters)

### Client (Station) Mode
- Device connects to existing WiFi network
- Gets IP from DHCP
- Useful for internet access

**To switch to Client mode:**
1. Press `1` (WiFi Configuration)
2. Press `3` (Configure Client Credentials)
3. Enter WiFi SSID
4. Enter WiFi password
5. Press `1` again, then `1` to select Client mode

## CAN Bus Configuration

### Setting CAN Speed

Both CAN channels can be configured independently.

**Supported speeds:**
- 10 kbps
- 20 kbps
- 50 kbps
- 100 kbps
- 125 kbps
- 250 kbps
- 500 kbps (default)
- 1000 kbps

**To configure CAN speed:**
1. Press `2` (CAN Bus Configuration)
2. Press `1` for CAN0 or `2` for CAN1
3. Select desired speed from menu

## CAN Bus Sniffing

The sniffer displays all CAN messages in real-time.

**Message format:**
```
[timestamp] [CANx] ID [DLC] DATA
```

**Example:**
```
[12345] [CAN0] 00000123 [8] 01 02 03 04 05 06 07 08
```

**To start sniffing:**
1. Press `3` (CAN Sniffer)
2. Press `1` for CAN0 or `2` for CAN1
3. Messages will stream to serial monitor

**To stop sniffing:**
1. Press `3` (CAN Sniffer)
2. Press `1` (Stop Sniffer)

## CANopen Joystick

### Configuring a Joystick

1. Press `4` (CANopen Joystick)
2. Press `1` for CAN0 or `2` for CAN1
3. Enter the joystick's CANopen node ID (1-127)
4. System will enable PDO for that node

### Testing a Joystick

1. Press `4` (CANopen Joystick)
2. Press `3` for CAN0 or `4` for CAN1
3. Enter the joystick's node ID
4. Joystick data will stream to serial monitor
5. Press any key to stop

**Joystick data format:**
```
[Joystick 1] X: 12345, Y: -6789, Z: 0, Buttons: 0x0003
```

- **X, Y, Z**: Axis values (typically -32768 to +32767)
- **Buttons**: Button states as hexadecimal bitmask

## View Status

Press `5` to view current configuration:
- WiFi mode and IP address
- CAN bus speeds and status
- Sniffer status

## Reset to Defaults

Press `6` to reset all settings:
- WiFi returns to AP mode with default credentials
- CAN speeds reset to 500 kbps
- Device will automatically reboot

## Tips and Best Practices

1. **Always match CAN speed**: Ensure all devices on the same CAN bus use the same baud rate

2. **Node ID conflicts**: Each CANopen device needs a unique node ID

3. **Serial buffer**: If using sniffer on high-traffic bus, messages may be lost due to serial speed limitations

4. **Power cycle**: If CAN bus doesn't work after changing speed, try power cycling the entire CAN network

5. **Save settings**: All configuration changes are automatically saved to flash memory

6. **Web interface**: Future versions may include web configuration interface accessible via WiFi IP address
