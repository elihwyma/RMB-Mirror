#include <cstdio>
#include <servo_control.h>
#include <filesystem>
#include <sys/types.h>
#include <cmath>

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

#define BAUDRATE                        115200

#define TORQUE_ENABLE                   1                   // Value for enabling the torque
#define TORQUE_DISABLE                  0                   // Value for disabling the torque
#define DXL_MINIMUM_POSITION_VALUE      0                 // Dynamixel will rotate between this value
#define DXL_MAXIMUM_POSITION_VALUE      1023                // and this value (note that the Dynamixel would not move when the position value is out of movable range. Check e-manual about the range of the Dynamixel you use.)
#define DXL_MOVING_STATUS_THRESHOLD     2                  // Dynamixel moving status threshold

using namespace dynamixel;

float currentx;
float currenty;

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

    if (openPort() != 0) {
        fprintf(stderr, "Failed to open port\n");
        exit(1);
    }

    for (uint8_t id = 1; id <= 3; id++) {
        if (setPositionMode(id) != 0) {
            fprintf(stderr, "Failed to set wheel mode on %d. It may not be connected.\n", id);
            exit(1);
        }
    }
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
        fprintf(stdout, "Automatically Detected Port: %s\n", port_name->c_str());
    } else {
        fprintf(stderr, "No available port found\n");
        exit(1);
    }
}

int16_t ServoControl::setPosition(uint8_t id, uint16_t position) {
  int ret = write2ByteTxRx(id, ADDR_MX_GOAL_POSITION, position);
  if (ret != 0) {
    fprintf(stderr, "Failed to set position\n");
    return -1;
  }
  int16_t currentPosition = 0;
  do {
    currentPosition = getPosition(id);
    if (currentPosition == -1) {
      return -1;
    }
  } while (abs(position - currentPosition) > DXL_MOVING_STATUS_THRESHOLD);
  return 0;
}

int16_t ServoControl::read2ByteTxRx(uint8_t id, uint16_t address, uint16_t *data) {
  uint8_t dxl_error = 0;
  int dxl_comm_result = packetHandler->read2ByteTxRx(portHandler, id, address, data, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS) {
    printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
    return -1;
  } else if (dxl_error != 0) {
    printf("%s %d\n", packetHandler->getRxPacketError(dxl_error), dxl_error);

    closePort();
    openPort();

    return -1;
  }
  return 0;
}

int16_t ServoControl::write2ByteTxRx(uint8_t id, uint16_t address, uint16_t data) {
  uint8_t dxl_error = 0;
  int dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, id, address, data, &dxl_error);
  if (dxl_comm_result != COMM_SUCCESS) {
    printf("%s\n", packetHandler->getTxRxResult(dxl_comm_result));
    return -1;
  } else if (dxl_error != 0) {
    printf("%s %d\n", packetHandler->getRxPacketError(dxl_error), dxl_error);

    closePort();
    openPort();

    return -1;
  }
  return 0;
}

int16_t ServoControl::getPosition(uint8_t id) {
  uint16_t position = 0;
  int16_t ret = read2ByteTxRx(id, ADDR_MX_PRESENT_POSITION, &position);
  if (ret != 0) {
    return -1;
  }
  return position;
}

uint8_t ServoControl::closePort() {
  portHandler->closePort();
  return 0;
}

uint8_t ServoControl::openPort() {
  uint8_t ret = portHandler->openPort();
  if (!ret) {
    printf("Failed to open the port!\n");
    return 1;
  }
  ret = portHandler->setBaudRate(BAUDRATE);
  if (!ret) {
    printf("Failed to change the baudrate!\n");
    return 2;
  }
  return 0;
}

int16_t ServoControl::setWheelMode(uint8_t id) {
  int ret = write2ByteTxRx(id, 6, 0);
  if (ret != 0) {
    fprintf(stderr, "Failed to set wheel mode\n");
    return -1;
  }
  ret = write2ByteTxRx(id, 8, 0);
  if (ret != 0) {
    fprintf(stderr, "Failed to set wheel mode\n");
    return -1;
  }
  return 0;
}

int16_t ServoControl::setPositionMode(uint8_t id) {
  int ret = write2ByteTxRx(id, 6, 0);
  if (ret != 0) {
    fprintf(stderr, "Failed to set position mode\n");
    return -1;
  }
  ret = write2ByteTxRx(id, 8, 1023);
  if (ret != 0) {
    fprintf(stderr, "Failed to set position mode\n");
    return -1;
  }
  return 0;
}

int16_t ServoControl::setWheelSpeed(uint8_t id, uint8_t direction, uint16_t speed) {
  // 10th bit determines direction
  if (direction == 1) {
    speed = speed | (1 << 10);
  } else {
    speed = speed & ~(1 << 10);
  }
  int ret = write2ByteTxRx(id, 32, speed);
  if (ret != 0) {
    fprintf(stderr, "Failed to set wheel speed\n");
    return -1;
  }
  return 0;
}

#define sq(x) ((x)*(x))

void ServoControl::InverseKinematics(float x, float y) {
  float l1 = 110;
  float l2 = 135;
  float angle1, angle2, rad_angle1, rad_angle2, dmx_value1, dmx_value2;
  float pi = 3.1415926535897932384626433832795;
  //Calculate IK & convert to radians - rad_angle2 = beta, rad_angle1 = alpha
  rad_angle2 = acos( (sq(x) + sq(y) - sq(l1) - sq(l2) ) / (2.0 * l1 * l2) );
  rad_angle1 = atan2(y, x) - atan2(l2 * sin(- rad_angle2), l1 + l2 * cos(- rad_angle2) );
  //Convert to degrees
  angle1 = rad_angle1 * (180 / pi);
  angle2 = rad_angle2 * (180 / pi);
  //Convert to dynamixel range
  dmx_value1 = angle1 * (1023/360);
  dmx_value2 = angle2 * (1023/360);
  //Apply degrees to servos
  setPosition(2, dmx_value1);
  setPosition(1, dmx_value2);
}

void ServoControl::Interpolate(float targetx, float targety) {
  /*
  if (abs(targetx - currentx) > abs(targety - currenty)){
    if (currentx < targetx){
      for (int x = currentx; x < targetx; x + 0.2){
        float y = m * x + c;
        InverseKinematics(x, y);
      }
    }
    else (currentx > targetx){
      for (int x = currentx; x > targetx; x - 0.2){
        float y = m * x + c;
        InverseKinematics(x, y);
      }
    }
  }
  else{
    if (currenty < targety){
      for (int y = currenty; y < targety; y + 0.2){
        float x = (y - c)/m;
        InverseKinematics(x, y);
      }
    } else if (currenty > targety) {
      for (int y = currenty; y > targety; y - 0.2){
        float x = (y - c)/m;
        InverseKinematics(x, y);
      }
    }
  }
    */
}
