#include "CANConfig.h"

// ===== CANBusChannel Implementation =====

CANBusChannel::CANBusChannel(uint8_t channel, gpio_num_t tx, gpio_num_t rx)
    : channelNum(channel), txPin(tx), rxPin(rx), currentSpeed(DEFAULT_CAN_SPEED), running(false) {
}

can_timing_config_t CANBusChannel::getTimingConfig(CANSpeed speed) {
    can_timing_config_t timing_config;

    switch (speed) {
        case CAN_SPEED_10KBPS:
            timing_config = CAN_TIMING_CONFIG_10KBITS();
            break;
        case CAN_SPEED_20KBPS:
            timing_config = CAN_TIMING_CONFIG_20KBITS();
            break;
        case CAN_SPEED_50KBPS:
            timing_config = CAN_TIMING_CONFIG_50KBITS();
            break;
        case CAN_SPEED_100KBPS:
            timing_config = CAN_TIMING_CONFIG_100KBITS();
            break;
        case CAN_SPEED_125KBPS:
            timing_config = CAN_TIMING_CONFIG_125KBITS();
            break;
        case CAN_SPEED_250KBPS:
            timing_config = CAN_TIMING_CONFIG_250KBITS();
            break;
        case CAN_SPEED_1000KBPS:
            timing_config = CAN_TIMING_CONFIG_1MBITS();
            break;
        case CAN_SPEED_500KBPS:
        default:
            timing_config = CAN_TIMING_CONFIG_500KBITS();
            break;
    }

    return timing_config;
}

bool CANBusChannel::begin(CANSpeed speed) {
    currentSpeed = speed;

    // General configuration
    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(txPin, rxPin, CAN_MODE_NORMAL);
    g_config.tx_queue_len = 20;
    g_config.rx_queue_len = 20;

    // Timing configuration
    can_timing_config_t t_config = getTimingConfig(speed);

    // Filter configuration (accept all)
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    // Install CAN driver
    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        // Start CAN driver
        if (can_start() == ESP_OK) {
            running = true;
            Serial.printf("CAN%d: Started at %d kbps\n", channelNum, speed);
            return true;
        }
    }

    Serial.printf("ERROR: Failed to start CAN%d\n", channelNum);
    return false;
}

void CANBusChannel::stop() {
    if (running) {
        can_stop();
        can_driver_uninstall();
        running = false;
        Serial.printf("CAN%d: Stopped\n", channelNum);
    }
}

bool CANBusChannel::isRunning() {
    return running;
}

bool CANBusChannel::sendMessage(uint32_t id, uint8_t* data, uint8_t len, bool extended) {
    if (!running) return false;

    can_message_t message;
    message.identifier = id;
    message.data_length_code = len;
    message.flags = extended ? CAN_MSG_FLAG_EXTD : CAN_MSG_FLAG_NONE;

    for (int i = 0; i < len && i < 8; i++) {
        message.data[i] = data[i];
    }

    return (can_transmit(&message, pdMS_TO_TICKS(100)) == ESP_OK);
}

bool CANBusChannel::receiveMessage(CANMessage& msg, uint32_t timeout) {
    if (!running) return false;

    can_message_t can_msg;
    esp_err_t result = can_receive(&can_msg, pdMS_TO_TICKS(timeout));

    if (result == ESP_OK) {
        msg.id = can_msg.identifier;
        msg.dlc = can_msg.data_length_code;
        msg.extended = (can_msg.flags & CAN_MSG_FLAG_EXTD) != 0;
        msg.rtr = (can_msg.flags & CAN_MSG_FLAG_RTR) != 0;
        msg.timestamp = millis();

        for (int i = 0; i < msg.dlc && i < 8; i++) {
            msg.data[i] = can_msg.data[i];
        }

        return true;
    }

    return false;
}

void CANBusChannel::setSpeed(CANSpeed speed) {
    if (speed != currentSpeed) {
        bool wasRunning = running;
        if (wasRunning) {
            stop();
        }
        if (wasRunning) {
            begin(speed);
        } else {
            currentSpeed = speed;
        }
    }
}

CANSpeed CANBusChannel::getSpeed() {
    return currentSpeed;
}

void CANBusChannel::printMessage(const CANMessage& msg) {
    Serial.print(messageToString(msg));
}

String CANBusChannel::messageToString(const CANMessage& msg) {
    char buffer[128];
    sprintf(buffer, "[CAN%d] %08X [%d] ", channelNum, msg.id, msg.dlc);
    String result = String(buffer);

    for (int i = 0; i < msg.dlc; i++) {
        sprintf(buffer, "%02X ", msg.data[i]);
        result += String(buffer);
    }

    if (msg.extended) result += " (EXT)";
    if (msg.rtr) result += " (RTR)";

    return result;
}

// ===== CANConfig Implementation =====

CANConfig::CANConfig() {
    sniffingEnabled = false;
    sniffingChannel = 0;
}

void CANConfig::begin() {
    preferences.begin("can-config", false);

    // Initialize CAN channels
    can0 = new CANBusChannel(0, CAN0_TX_PIN, CAN0_RX_PIN);
    can1 = new CANBusChannel(1, CAN1_TX_PIN, CAN1_RX_PIN);

    loadConfig();

    // Start both CAN channels with configured speeds
    can0->begin(getSpeed(0));
    can1->begin(getSpeed(1));

    Serial.println("CAN bus initialized");
}

void CANConfig::loadConfig() {
    // Load CAN speeds
    uint32_t speed0 = preferences.getUInt("can0_speed", DEFAULT_CAN_SPEED);
    uint32_t speed1 = preferences.getUInt("can1_speed", DEFAULT_CAN_SPEED);

    can0->setSpeed((CANSpeed)speed0);
    can1->setSpeed((CANSpeed)speed1);

    Serial.println("CAN configuration loaded");
}

void CANConfig::saveConfig() {
    preferences.putUInt("can0_speed", can0->getSpeed());
    preferences.putUInt("can1_speed", can1->getSpeed());

    Serial.println("CAN configuration saved");
}

void CANConfig::resetToDefaults() {
    preferences.clear();
    can0->setSpeed(DEFAULT_CAN_SPEED);
    can1->setSpeed(DEFAULT_CAN_SPEED);
    saveConfig();
    Serial.println("CAN configuration reset to defaults");
}

CANBusChannel* CANConfig::getCAN0() {
    return can0;
}

CANBusChannel* CANConfig::getCAN1() {
    return can1;
}

void CANConfig::setSpeed(uint8_t channel, CANSpeed speed) {
    if (channel == 0) {
        can0->setSpeed(speed);
    } else if (channel == 1) {
        can1->setSpeed(speed);
    }
    saveConfig();
}

CANSpeed CANConfig::getSpeed(uint8_t channel) {
    if (channel == 0) {
        return can0->getSpeed();
    } else if (channel == 1) {
        return can1->getSpeed();
    }
    return DEFAULT_CAN_SPEED;
}

void CANConfig::startSniffing(uint8_t channel) {
    if (channel <= 1) {
        sniffingEnabled = true;
        sniffingChannel = channel;
        Serial.printf("Started sniffing on CAN%d\n", channel);
    }
}

void CANConfig::stopSniffing() {
    sniffingEnabled = false;
    Serial.println("Stopped sniffing");
}

bool CANConfig::isSniffing() {
    return sniffingEnabled;
}

uint8_t CANConfig::getSniffingChannel() {
    return sniffingChannel;
}

void CANConfig::printStatus() {
    Serial.println("\n=== CAN Bus Status ===");
    Serial.printf("CAN0: %s, Speed: %d kbps\n",
                  can0->isRunning() ? "Running" : "Stopped",
                  can0->getSpeed());
    Serial.printf("CAN1: %s, Speed: %d kbps\n",
                  can1->isRunning() ? "Running" : "Stopped",
                  can1->getSpeed());

    if (sniffingEnabled) {
        Serial.printf("Sniffing: CAN%d\n", sniffingChannel);
    }

    Serial.println("======================\n");
}
