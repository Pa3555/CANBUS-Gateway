#include "CANopen.h"

CANopen::CANopen(CANBusChannel* canChannel) : can(canChannel), lastJoystickNodeId(0) {
}

void CANopen::begin() {
    Serial.println("CANopen initialized");
}

void CANopen::process() {
    // Process incoming CANopen messages
    CANMessage msg;
    if (can->receiveMessage(msg, 0)) {
        uint8_t nodeId;
        uint16_t functionCode;
        extractNodeIdAndFunction(msg.id, nodeId, functionCode);

        // Process different message types
        if (functionCode >= CANOPEN_TPDO1 && functionCode < CANOPEN_TPDO1 + 0x80) {
            // TPDO message - could be joystick data
            // Handle in readJoystick() method
        }
    }
}

uint32_t CANopen::makeCANId(uint16_t functionCode, uint8_t nodeId) {
    return functionCode + nodeId;
}

void CANopen::extractNodeIdAndFunction(uint32_t canId, uint8_t& nodeId, uint16_t& functionCode) {
    nodeId = canId & 0x7F;
    functionCode = canId & 0x780;
}

// ===== NMT Management =====

void CANopen::sendNMT(uint8_t nodeId, uint8_t command) {
    uint8_t data[2];
    data[0] = command;
    data[1] = nodeId;
    can->sendMessage(CANOPEN_NMT, data, 2, false);
}

void CANopen::startNode(uint8_t nodeId) {
    sendNMT(nodeId, NMT_CMD_START);
    Serial.printf("CANopen: Started node %d\n", nodeId);
}

void CANopen::stopNode(uint8_t nodeId) {
    sendNMT(nodeId, NMT_CMD_STOP);
    Serial.printf("CANopen: Stopped node %d\n", nodeId);
}

void CANopen::resetNode(uint8_t nodeId) {
    sendNMT(nodeId, NMT_CMD_RESET_NODE);
    Serial.printf("CANopen: Reset node %d\n", nodeId);
}

// ===== SDO Operations =====

bool CANopen::sdoDownload(uint8_t nodeId, uint16_t index, uint8_t subIndex, uint32_t data, uint8_t dataSize) {
    uint8_t sdo[8];

    // Expedited download
    sdo[0] = SDO_CMD_DOWNLOAD_INIT | ((4 - dataSize) << 2) | 0x03; // expedited, size indicated
    sdo[1] = index & 0xFF;
    sdo[2] = (index >> 8) & 0xFF;
    sdo[3] = subIndex;

    // Data (little-endian)
    for (int i = 0; i < 4; i++) {
        sdo[4 + i] = (data >> (i * 8)) & 0xFF;
    }

    uint32_t cobId = makeCANId(CANOPEN_SDO_RX, nodeId);
    can->sendMessage(cobId, sdo, 8, false);

    // Wait for response
    SDOResponse response;
    return waitForSDOResponse(nodeId, response, 100);
}

bool CANopen::sdoUpload(uint8_t nodeId, uint16_t index, uint8_t subIndex, SDOResponse& response) {
    uint8_t sdo[8] = {0};

    // Upload request
    sdo[0] = SDO_CMD_UPLOAD_INIT;
    sdo[1] = index & 0xFF;
    sdo[2] = (index >> 8) & 0xFF;
    sdo[3] = subIndex;

    uint32_t cobId = makeCANId(CANOPEN_SDO_RX, nodeId);
    can->sendMessage(cobId, sdo, 8, false);

    // Wait for response
    return waitForSDOResponse(nodeId, response, 100);
}

bool CANopen::waitForSDOResponse(uint8_t nodeId, SDOResponse& response, uint32_t timeout) {
    uint32_t startTime = millis();
    uint32_t expectedCobId = makeCANId(CANOPEN_SDO_TX, nodeId);

    while (millis() - startTime < timeout) {
        CANMessage msg;
        if (can->receiveMessage(msg, 10)) {
            if (msg.id == expectedCobId && msg.dlc == 8) {
                uint8_t cmd = msg.data[0];

                if (cmd & 0x80) {
                    // Abort
                    response.success = false;
                    Serial.printf("SDO Abort: Node %d, Error code: 0x%08X\n",
                                nodeId,
                                (msg.data[7] << 24) | (msg.data[6] << 16) |
                                (msg.data[5] << 8) | msg.data[4]);
                    return false;
                }

                // Success response
                response.nodeId = nodeId;
                response.index = msg.data[1] | (msg.data[2] << 8);
                response.subIndex = msg.data[3];
                response.data = msg.data[4] | (msg.data[5] << 8) |
                               (msg.data[6] << 16) | (msg.data[7] << 24);
                response.success = true;

                // Determine data size from command byte
                if (cmd & 0x02) {
                    response.dataSize = 4 - ((cmd >> 2) & 0x03);
                } else {
                    response.dataSize = 4;
                }

                return true;
            }
        }
    }

    response.success = false;
    return false;
}

// ===== PDO Operations =====

void CANopen::configurePDO(uint8_t nodeId, uint8_t pdoNum, uint16_t cobId, uint8_t transmissionType) {
    // Configure PDO communication parameter
    uint16_t commIndex = 0x1400 + (pdoNum - 1); // RPDO
    if (pdoNum > 4) {
        commIndex = 0x1800 + (pdoNum - 5); // TPDO
    }

    // Set COB-ID
    sdoDownload(nodeId, commIndex, 1, cobId, 4);

    // Set transmission type
    sdoDownload(nodeId, commIndex, 2, transmissionType, 1);

    Serial.printf("CANopen: Configured PDO%d for node %d\n", pdoNum, nodeId);
}

void CANopen::sendTPDO(uint8_t nodeId, uint8_t pdoNum, uint8_t* data, uint8_t len) {
    uint16_t functionCode = CANOPEN_TPDO1 + ((pdoNum - 1) * 0x100);
    uint32_t cobId = makeCANId(functionCode, nodeId);
    can->sendMessage(cobId, data, len, false);
}

bool CANopen::receiveRPDO(uint8_t nodeId, uint8_t pdoNum, uint8_t* data, uint8_t& len) {
    uint16_t functionCode = CANOPEN_RPDO1 + ((pdoNum - 1) * 0x100);
    uint32_t expectedCobId = makeCANId(functionCode, nodeId);

    CANMessage msg;
    if (can->receiveMessage(msg, 0)) {
        if (msg.id == expectedCobId) {
            len = msg.dlc;
            for (int i = 0; i < msg.dlc && i < 8; i++) {
                data[i] = msg.data[i];
            }
            return true;
        }
    }

    return false;
}

// ===== Joystick Specific =====

void CANopen::setJoystickNodeId(uint8_t nodeId) {
    lastJoystickNodeId = nodeId;
}

void CANopen::enableJoystickPDO(uint8_t nodeId) {
    // Start node in operational state
    startNode(nodeId);

    // Configure TPDO1 for joystick data (typically sent automatically)
    // Most CANopen joysticks send data on TPDO1 by default

    Serial.printf("CANopen: Enabled joystick on node %d\n", nodeId);
}

bool CANopen::readJoystick(uint8_t nodeId, JoystickData& joystick) {
    // Read TPDO1 from joystick
    uint16_t functionCode = CANOPEN_TPDO1;
    uint32_t expectedCobId = makeCANId(functionCode, nodeId);

    CANMessage msg;
    if (can->receiveMessage(msg, 0)) {
        if (msg.id == expectedCobId && msg.dlc >= 6) {
            // Parse joystick data (format may vary by manufacturer)
            // Typical format: 2 bytes X, 2 bytes Y, 2 bytes Z/buttons
            joystick.axisX = (int16_t)(msg.data[0] | (msg.data[1] << 8));
            joystick.axisY = (int16_t)(msg.data[2] | (msg.data[3] << 8));

            if (msg.dlc >= 8) {
                joystick.axisZ = (int16_t)(msg.data[4] | (msg.data[5] << 8));
                joystick.buttons = msg.data[6] | (msg.data[7] << 8);
            } else {
                joystick.axisZ = 0;
                joystick.buttons = msg.data[4] | (msg.data[5] << 8);
            }

            joystick.nodeId = nodeId;
            joystick.timestamp = msg.timestamp;

            return true;
        }
    }

    return false;
}

void CANopen::printJoystickData(const JoystickData& joystick) {
    Serial.printf("[Joystick %d] X: %6d, Y: %6d, Z: %6d, Buttons: 0x%04X\n",
                  joystick.nodeId,
                  joystick.axisX,
                  joystick.axisY,
                  joystick.axisZ,
                  joystick.buttons);
}

// ===== Heartbeat =====

void CANopen::sendHeartbeat(uint8_t nodeId, uint8_t state) {
    uint32_t cobId = makeCANId(CANOPEN_HEARTBEAT, nodeId);
    can->sendMessage(cobId, &state, 1, false);
}
