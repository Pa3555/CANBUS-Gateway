# CANopen Reference Guide

## What is CANopen?

CANopen is a higher-layer protocol built on top of CAN bus, providing standardized communication for industrial automation and embedded systems. It's commonly used in:
- Industrial joysticks and control panels
- Motor controllers
- I/O modules
- Sensors and actuators

## CANopen Communication Objects

### 1. NMT (Network Management)
- **COB-ID**: 0x000
- **Purpose**: Control node states
- **States**: Boot-up, Pre-operational, Operational, Stopped

### 2. SDO (Service Data Object)
- **COB-ID RX**: 0x600 + Node ID
- **COB-ID TX**: 0x580 + Node ID
- **Purpose**: Read/write object dictionary entries
- **Usage**: Configuration, parameter access

### 3. PDO (Process Data Object)
- **RPDO** (Receive): 0x200, 0x300, 0x400, 0x500 + Node ID
- **TPDO** (Transmit): 0x180, 0x280, 0x380, 0x480 + Node ID
- **Purpose**: Real-time data exchange
- **Usage**: Joystick axes, buttons, sensor data

### 4. SYNC
- **COB-ID**: 0x080
- **Purpose**: Synchronize PDO transmission

### 5. Emergency
- **COB-ID**: 0x080 + Node ID
- **Purpose**: Error messages

### 6. Heartbeat
- **COB-ID**: 0x700 + Node ID
- **Purpose**: Node alive indication

## Common Object Dictionary Indices

### Device Control
- **0x1000**: Device Type
- **0x1001**: Error Register
- **0x1017**: Producer Heartbeat Time

### PDO Communication Parameters
- **0x1400-0x15FF**: RPDO Communication Parameters
- **0x1800-0x19FF**: TPDO Communication Parameters

### PDO Mapping Parameters
- **0x1600-0x17FF**: RPDO Mapping Parameters
- **0x1A00-0x1BFF**: TPDO Mapping Parameters

## Joystick Integration

### Typical Joystick PDO Configuration

Most CANopen joysticks send data via TPDO1:

**CAN ID**: 0x180 + Node ID

**Data Layout** (8 bytes):
```
Byte 0-1: X-axis (signed 16-bit)
Byte 2-3: Y-axis (signed 16-bit)
Byte 4-5: Z-axis/rotation (signed 16-bit)
Byte 6-7: Button states (16 bits)
```

### Example: Reading Joystick with Node ID 5

**CAN Message:**
```
ID: 0x185 (0x180 + 5)
DLC: 8
Data: 10 00 F0 FF 00 00 03 00
```

**Interpretation:**
- X-axis: 0x0010 = 16
- Y-axis: 0xFFF0 = -16
- Z-axis: 0x0000 = 0
- Buttons: 0x0003 = buttons 0 and 1 pressed

### Setting Up a Joystick

1. **Configure Node ID** on the joystick (via DIP switches or software)

2. **Enable CANopen** in the gateway:
   ```
   Menu -> 4 (CANopen Joystick) -> 1 (Configure CAN0)
   Enter Node ID: 5
   ```

3. **Start Reading Data**:
   ```
   Menu -> 4 (CANopen Joystick) -> 3 (Test CAN0)
   Enter Node ID: 5
   ```

## SDO Communication Examples

### Reading a Parameter

**Request** (Upload Init):
```
COB-ID: 0x600 + Node ID
Data: 40 00 10 00 00 00 00 00
      ^  ^^^^^  ^
      |    |    |
      |    |    +-- Sub-index
      |    +------- Index (0x1000 = Device Type)
      +------------ Command (0x40 = Upload Init)
```

**Response**:
```
COB-ID: 0x580 + Node ID
Data: 43 00 10 00 91 00 01 00
      ^  ^^^^^  ^  ^^^^^^^^^^^
      |    |    |       |
      |    |    |       +------ Data (Device Type value)
      |    |    +-------------- Sub-index
      |    +------------------- Index
      +------------------------ Response (0x43 = Upload success)
```

### Writing a Parameter

**Request** (Download Init):
```
COB-ID: 0x600 + Node ID
Data: 23 17 10 00 E8 03 00 00
      ^  ^^^^^  ^  ^^^^^^^^^^^
      |    |    |       |
      |    |    |       +------ Data to write (0x03E8 = 1000ms)
      |    |    +-------------- Sub-index
      |    +------------------- Index (0x1017 = Heartbeat time)
      +------------------------ Command (0x23 = Download 2 bytes)
```

**Response**:
```
COB-ID: 0x580 + Node ID
Data: 60 17 10 00 00 00 00 00
      ^
      +-- Response (0x60 = Download success)
```

## Common Joystick Vendors

### APEM
- Typically uses Node IDs 1-16
- TPDO1 for axis data
- TPDO2 for additional buttons

### Otto
- Configurable Node ID via DIP switches
- Standard CANopen profile
- Multiple PDO configurations

### Parker
- Industrial-grade joysticks
- Extended button support
- May use custom object dictionary

## Troubleshooting

### No Data from Joystick

1. **Check Node ID**: Ensure correct Node ID configured
2. **Check NMT State**: Joystick must be in Operational state
3. **Check PDO Configuration**: Verify TPDO is enabled
4. **Check CAN Speed**: Must match joystick baud rate
5. **Check Wiring**: Verify CAN_H, CAN_L, and termination

### Incorrect Data Values

1. **Byte Order**: Check if joystick uses big-endian or little-endian
2. **Scaling**: Some joysticks scale values differently
3. **Data Mapping**: Verify PDO mapping in object dictionary

### Communication Errors

1. **Bus Off**: Check termination and cable quality
2. **ACK Errors**: Ensure at least 2 nodes on bus
3. **Timeout**: Increase SDO timeout if network is slow

## Additional Resources

- **CiA 301**: CANopen Application Layer and Communication Profile
- **CiA 401**: CANopen Device Profile for I/O Modules (includes joysticks)
- **Manufacturer Documentation**: Always check joystick-specific documentation
