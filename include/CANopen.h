#ifndef CANOPEN_H
#define CANOPEN_H

#include <Arduino.h>
#include "CANConfig.h"

// CANopen function codes
#define CANOPEN_NMT         0x000
#define CANOPEN_SYNC        0x080
#define CANOPEN_EMERGENCY   0x080
#define CANOPEN_TIMESTAMP   0x100
#define CANOPEN_TPDO1       0x180
#define CANOPEN_RPDO1       0x200
#define CANOPEN_TPDO2       0x280
#define CANOPEN_RPDO2       0x300
#define CANOPEN_TPDO3       0x380
#define CANOPEN_RPDO3       0x400
#define CANOPEN_TPDO4       0x480
#define CANOPEN_RPDO4       0x500
#define CANOPEN_SDO_TX      0x580
#define CANOPEN_SDO_RX      0x600
#define CANOPEN_HEARTBEAT   0x700

// SDO commands
#define SDO_CMD_DOWNLOAD_INIT       0x20
#define SDO_CMD_DOWNLOAD_SEGMENT    0x00
#define SDO_CMD_UPLOAD_INIT         0x40
#define SDO_CMD_UPLOAD_SEGMENT      0x60
#define SDO_CMD_ABORT               0x80

// SDO response commands
#define SDO_RESP_DOWNLOAD_INIT      0x60
#define SDO_RESP_DOWNLOAD_SEGMENT   0x20
#define SDO_RESP_UPLOAD_INIT        0x40
#define SDO_RESP_UPLOAD_SEGMENT     0x00
#define SDO_RESP_ABORT              0x80

// NMT commands
#define NMT_CMD_START       0x01
#define NMT_CMD_STOP        0x02
#define NMT_CMD_PREOP       0x80
#define NMT_CMD_RESET_NODE  0x81
#define NMT_CMD_RESET_COMM  0x82

// Joystick data structure (typical CANopen joystick)
struct JoystickData {
    int16_t axisX;      // X-axis value
    int16_t axisY;      // Y-axis value
    int16_t axisZ;      // Z-axis (twist/rotation)
    uint16_t buttons;   // Button states (bitfield)
    uint8_t nodeId;     // CANopen node ID
    uint32_t timestamp;
};

// SDO request structure
struct SDORequest {
    uint8_t nodeId;
    uint16_t index;
    uint8_t subIndex;
    uint32_t data;
    uint8_t dataSize;
};

// SDO response structure
struct SDOResponse {
    uint8_t nodeId;
    uint16_t index;
    uint8_t subIndex;
    uint32_t data;
    uint8_t dataSize;
    bool success;
};

class CANopen {
public:
    CANopen(CANBusChannel* canChannel);

    void begin();
    void process();

    // NMT management
    void sendNMT(uint8_t nodeId, uint8_t command);
    void startNode(uint8_t nodeId);
    void stopNode(uint8_t nodeId);
    void resetNode(uint8_t nodeId);

    // SDO operations
    bool sdoDownload(uint8_t nodeId, uint16_t index, uint8_t subIndex, uint32_t data, uint8_t dataSize);
    bool sdoUpload(uint8_t nodeId, uint16_t index, uint8_t subIndex, SDOResponse& response);

    // PDO operations
    void configurePDO(uint8_t nodeId, uint8_t pdoNum, uint16_t cobId, uint8_t transmissionType);
    void sendTPDO(uint8_t nodeId, uint8_t pdoNum, uint8_t* data, uint8_t len);
    bool receiveRPDO(uint8_t nodeId, uint8_t pdoNum, uint8_t* data, uint8_t& len);

    // Joystick specific
    bool readJoystick(uint8_t nodeId, JoystickData& joystick);
    void setJoystickNodeId(uint8_t nodeId);
    void enableJoystickPDO(uint8_t nodeId);
    void printJoystickData(const JoystickData& joystick);

    // Heartbeat
    void sendHeartbeat(uint8_t nodeId, uint8_t state);

private:
    CANBusChannel* can;
    uint8_t lastJoystickNodeId;

    // Helper functions
    uint32_t makeCANId(uint16_t functionCode, uint8_t nodeId);
    void extractNodeIdAndFunction(uint32_t canId, uint8_t& nodeId, uint16_t& functionCode);
    bool waitForSDOResponse(uint8_t nodeId, SDOResponse& response, uint32_t timeout = 100);
};

#endif // CANOPEN_H
