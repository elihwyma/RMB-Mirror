#include <servo_control.h>
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
#elif defined(__APPLE__) || defined(__linux__)
    #include <dirent.h>
#endif

// Control table address
#define ADDR_MX_TORQUE_ENABLE           24                  // Control table address is different in Dynamixel model
#define ADDR_MX_GOAL_POSITION           30
#define ADDR_MX_PRESENT_POSITION        36

// Protocol version
#define PROTOCOL_VERSION                1.0                 // See which protocol version is used in the Dynamixel

// Default setting
#define DXL_ID                          1                   // Dynamixel ID: 1
#define BAUDRATE                        1000000
#define DEVICENAME                      "/dev/ttyUSB0"      // Check which port is being used on your controller
                                                            // ex) Windows: "COM1"   Linux: "/dev/ttyUSB0" Mac: "/dev/tty.usbserial-*"

#define TORQUE_ENABLE                   1                   // Value for enabling the torque
#define TORQUE_DISABLE                  0                   // Value for disabling the torque
#define DXL_MINIMUM_POSITION_VALUE      0                 // Dynamixel will rotate between this value
#define DXL_MAXIMUM_POSITION_VALUE      1023                // and this value (note that the Dynamixel would not move when the position value is out of movable range. Check e-manual about the range of the Dynamixel you use.)
#define DXL_MOVING_STATUS_THRESHOLD     10                  // Dynamixel moving status threshold

using namespace dynamixel;

std::string ServoControl::errorDescription(int error) {
    return "";
}

ServoControl::ServoControl() {
    std::string port_name;
    getPortName(&port_name);

    PortHandler *portHandler = PortHandler::getPortHandler(port_name.c_str());
    PacketHandler *packetHandler = PacketHandler::getPacketHandler(PROTOCOL_VERSION);

    this->portHandler = portHandler;
    this->packetHandler = packetHandler;

    int index = 0;
    int dxl_comm_result = COMM_TX_FAIL;             // Communication result
    int dxl_goal_position[2] = {DXL_MINIMUM_POSITION_VALUE, DXL_MAXIMUM_POSITION_VALUE};         // Goal position

    uint8_t dxl_error = 0;                          // Dynamixel error
    uint16_t dxl_present_position = 0;              // Present position

    // Open port
    if (portHandler->openPort()) {
        printf("Succeeded to open the port!\n");
    } else {
        fprintf(stderr, "Failed to open the port!\n");
        exit(1);
    }

    
    // Set port baudrate
    if (portHandler->setBaudRate(115200))
    {
        printf("Succeeded to change the baudrate!\n");
    }
    else
    {
        fprintf(stderr, "Failed to change the baudrate!\n");
        exit(1);
    }
    /*
    while(1)
  {
    printf("Press any key to continue! (or press ESC to quit!)\n");
    if (getch() == ESC_ASCII_VALUE)
      break;

    // Write goal position
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID, ADDR_MX_GOAL_POSITION, dxl_goal_position[index], &dxl_error);
    dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, 2, ADDR_MX_GOAL_POSITION, dxl_goal_position[index], &dxl_error);
    if (dxl_comm_result != COMM_SUCCESS)
    {
      printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
    }
    else if (dxl_error != 0)
    {
      printf("%s\n", packetHandler->getRxPacketError(dxl_error));
    }

    do
    {
      // Read present position
      dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, DXL_ID, ADDR_MX_PRESENT_POSITION, &dxl_present_position, &dxl_error);
      if (dxl_comm_result != COMM_SUCCESS)
      {
        printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
      }
      else if (dxl_error != 0)
      {
        printf("%s\n", packetHandler->getRxPacketError(dxl_error));
      }

      printf("[ID:%03d] GoalPos:%03d  PresPos:%03d\n", DXL_ID, dxl_goal_position[index], dxl_present_position);

    }while((abs(dxl_goal_position[index] - dxl_present_position) > DXL_MOVING_STATUS_THRESHOLD));

    // Change goal position
    if (index == 0)
    {
      index = 1;
    }
    else
    {
      index = 0;
    }
  }
    */
}

void ServoControl::getPortName(std::string *port_name) {
    std::vector<std::string> possible_ports;

    #if defined(__APPLE__)
        std::string base_path = "/dev/";
        std::string prefix = "tty.usbserial-";
    #elif defined(_WIN32) || defined(_WIN64)
        for (int i = 1; i <= 256; ++i) {
            std::string comPort = "COM" + std::to_string(i);
            HANDLE hComm = CreateFile(comPort.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);
            if (hComm != INVALID_HANDLE_VALUE) {
                possible_ports.push_back(comPort);
                CloseHandle(hComm);
            }
        }
    #else // Linux
        std::string base_path = "/dev/";
        std::string prefix = "ttyUSB";
    #endif

    #if defined(__APPLE__) || defined(__linux__)
        for (const auto &entry : std::filesystem::directory_iterator(base_path)) {
            if (entry.path().filename().string().find(prefix) != std::string::npos) {
                possible_ports.push_back(entry.path().string());
            }
        }
    #endif

    if (!possible_ports.empty()) {
        *port_name = possible_ports[0]; // Select the first detected port
    } else {
        *port_name = ""; // No available port found
    }
}

int ServoControl::setPosition(uint8_t id, uint16_t position) {
    return 0;
}