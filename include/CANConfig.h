#ifndef CAN_CONFIG_H
#define CAN_CONFIG_H

#include <Arduino.h>
#include <Preferences.h>
#include <driver/gpio.h>
#include <driver/can.h>

// CAN bus speeds
enum CANSpeed {
    CAN_SPEED_10KBPS = 10,
    CAN_SPEED_20KBPS = 20,
    CAN_SPEED_50KBPS = 50,
    CAN_SPEED_100KBPS = 100,
    CAN_SPEED_125KBPS = 125,
    CAN_SPEED_250KBPS = 250,
    CAN_SPEED_500KBPS = 500,
    CAN_SPEED_1000KBPS = 1000
};

// Default GPIO pins for ESP32 CAN X2
#define CAN0_TX_PIN GPIO_NUM_5
#define CAN0_RX_PIN GPIO_NUM_4
#define CAN1_TX_PIN GPIO_NUM_25
#define CAN1_RX_PIN GPIO_NUM_26

// Default CAN speed
#define DEFAULT_CAN_SPEED CAN_SPEED_500KBPS

// CAN message structure
struct CANMessage {
    uint32_t id;
    uint8_t dlc;
    uint8_t data[8];
    bool extended;
    bool rtr;
    uint32_t timestamp;
};

class CANBusChannel {
public:
    CANBusChannel(uint8_t channel, gpio_num_t txPin, gpio_num_t rxPin);

    bool begin(CANSpeed speed);
    void stop();
    bool isRunning();

    bool sendMessage(uint32_t id, uint8_t* data, uint8_t len, bool extended = false);
    bool receiveMessage(CANMessage& msg, uint32_t timeout = 0);

    void setSpeed(CANSpeed speed);
    CANSpeed getSpeed();

    void printMessage(const CANMessage& msg);
    String messageToString(const CANMessage& msg);

private:
    uint8_t channelNum;
    gpio_num_t txPin;
    gpio_num_t rxPin;
    CANSpeed currentSpeed;
    bool running;

    can_timing_config_t getTimingConfig(CANSpeed speed);
};

class CANConfig {
public:
    CANConfig();
    void begin();
    void loadConfig();
    void saveConfig();
    void resetToDefaults();

    CANBusChannel* getCAN0();
    CANBusChannel* getCAN1();

    void setSpeed(uint8_t channel, CANSpeed speed);
    CANSpeed getSpeed(uint8_t channel);

    void startSniffing(uint8_t channel);
    void stopSniffing();
    bool isSniffing();
    uint8_t getSniffingChannel();

    void printStatus();

private:
    Preferences preferences;
    CANBusChannel* can0;
    CANBusChannel* can1;

    bool sniffingEnabled;
    uint8_t sniffingChannel;
};

#endif // CAN_CONFIG_H
