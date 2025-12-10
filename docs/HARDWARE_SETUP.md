# Hardware Setup Guide

## ESP32 CAN X2 from Autosport Labs

### Pin Configuration

#### CAN Channel 0
- **TX Pin**: GPIO 5
- **RX Pin**: GPIO 4

#### CAN Channel 1
- **TX Pin**: GPIO 25
- **RX Pin**: GPIO 26

### Wiring

#### CAN Bus Connection
Each CAN channel requires:
- **CAN_H**: High signal line
- **CAN_L**: Low signal line
- **GND**: Ground reference

#### Typical CAN Bus Termination
- 120Ω resistor between CAN_H and CAN_L at each end of the bus
- Maximum bus length depends on baud rate:
  - 1000 kbps: max 40m
  - 500 kbps: max 100m
  - 250 kbps: max 250m
  - 125 kbps: max 500m

### Power Supply
- **Input Voltage**: 5V via USB or VIN (7-12V)
- **Current**: Minimum 500mA recommended

## CANopen Joystick Connection

### Standard CANopen Network
1. Connect joystick to one of the CAN channels
2. Ensure proper termination (120Ω at both ends)
3. Power the joystick according to manufacturer specs
4. Set joystick node ID (typically via DIP switches or software)

### Typical Joystick Node IDs
- Node IDs range: 1-127
- Common default: 1 or 16
- Check your joystick documentation

## LED Indicators (if available on board)
- **Power LED**: Board is powered
- **CAN0 TX/RX**: Activity on CAN channel 0
- **CAN1 TX/RX**: Activity on CAN channel 1
- **WiFi LED**: WiFi connection status

## Troubleshooting Hardware

### CAN Bus Issues
- **No communication**: Check termination resistors
- **Intermittent errors**: Check cable quality and length
- **Bus off errors**: Check baud rate matches all devices

### WiFi Issues
- **No AP visible**: Check antenna connection
- **Poor signal**: Ensure antenna is properly connected
- **Cannot connect**: Verify credentials

### Power Issues
- **Random resets**: Increase power supply capacity
- **Brown-out**: Use regulated 5V supply with adequate current
