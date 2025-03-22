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
private:
    dynamixel::PortHandler *portHandler;
    dynamixel::PacketHandler *packetHandler;

    int16_t getPosition(uint8_t id);

    uint8_t closePort();
    uint8_t openPort();
};