// Heavily based c++/example/protocol1.0/read_write/read_write.cpp

#include <stdio.h>
#include <string>

#include <dynamixel_sdk.h>
class ServoControl {
public:
    ServoControl();

    static void getPortName(std::string *port_name);
    static std::string errorDescription(int error);
    int16_t setPosition(uint8_t id, uint16_t position);
    int16_t setWheelMode(uint8_t id);
    int16_t setPositionMode(uint8_t id);
    int16_t setWheelSpeed(uint8_t id, uint8_t direction, uint16_t speed);
private:
    dynamixel::PortHandler *portHandler;
    dynamixel::PacketHandler *packetHandler;

    int16_t getPosition(uint8_t id);
    int16_t read2ByteTxRx(uint8_t id, uint16_t address, uint16_t *data);
    int16_t write2ByteTxRx(uint8_t id, uint16_t address, uint16_t data);

    uint8_t closePort();
    uint8_t openPort();
};