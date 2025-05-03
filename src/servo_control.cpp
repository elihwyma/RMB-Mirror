#include <cstdint>
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

#define L1 165.0  // mm
#define L2 175.103  // mm
#define SERVO_MIN 0
#define SERVO_MAX 1023
#define SERVO_RANGE_DEGREES 300.0
#define SERVO_CENTER_DEGREE 150.0
#define sq(x) ((x)*(x))

#define ANGLE_TO_POSITION(x) \
  ((x) * (SERVO_MAX / SERVO_RANGE_DEGREES)) + (SERVO_MAX / 2)

#define RADIANS_TO_POSITION(x) \
  ((x) * (SERVO_MAX / (2 * M_PI))) + (SERVO_MAX / 2)

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

    int16_t error = 0;
    error = setPositionMode(1);
    if (error != 0) {
        fprintf(stderr, "Failed to set position mode on servo 1\n");
        exit(1);
    }
    error = setPositionMode(2);
    if (error != 0) {
        fprintf(stderr, "Failed to set position mode on servo 2\n");
        exit(1);
    }
    error = setWheelMode(3);
    if (error != 0) {
        fprintf(stderr, "Failed to set wheel mode on servo 3\n");
        exit(1);
    }
    fprintf(stdout, "Homing Servos...\n");
    setPair(512, 512);
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

int16_t ServoControl::setPair(uint16_t servoOne, uint16_t servoTwo) {
  // Safety checks that we're not about to kill ourself
  int ret = write2ByteTxRx(1, ADDR_MX_GOAL_POSITION, servoOne);
  if (ret != 0) {
    fprintf(stderr, "Failed to set position for servo 1\n");
    return -1;
  }
  ret = write2ByteTxRx(2, ADDR_MX_GOAL_POSITION, servoTwo);
  if (ret != 0) {
    fprintf(stderr, "Failed to set position for servo 2\n");
    return -1;
  }
  uint16_t status = 1;
  while (status) {
    int16_t error = read2ByteTxRx(1, 46, &status);
    if (error != 0) {
      fprintf(stderr, "Read error %d\n", error);
      return error;
    }
  }
  status = 1;
  while (status) {
    int16_t error = read2ByteTxRx(2, 46, &status);
    if (error != 0) {
      fprintf(stderr, "Read error %d\n", error);
      return error;
    }
  }
  return 0;
}

int16_t ServoControl::validatePositions(uint16_t servoOne, uint16_t servoTwo) {
  
  return 0;
}

int16_t ServoControl::setPosition(uint8_t id, uint16_t position) {
  int ret = write2ByteTxRx(id, ADDR_MX_GOAL_POSITION, position);
  if (ret != 0) {
    fprintf(stderr, "Failed to set position\n");
    return -1;
  }
  uint16_t status = 1;
  while (status) {
    int16_t error = read2ByteTxRx(id, 46, &status);
    if (error != 0) {
      fprintf(stderr, "Read error %d\n", error);
      return error;
    }
  }
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
    fprintf(stderr, "Failed to set position mode 1\n");
    return -1;
  }
  ret = write2ByteTxRx(id, 8, 1023);
  if (ret != 0) {
    fprintf(stderr, "Failed to set position mode 2\n");
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

double radToDeg(double radians) {
  return radians * (180.0 / M_PI);
}

uint16_t ServoControl::angleToPosition(double degrees) {
  // Map to 0 - 1023
  double pos = (degrees / SERVO_RANGE_DEGREES) * SERVO_MAX;
  // Clamp to valid servo range
  if (pos < SERVO_MIN) pos = SERVO_MIN;
  if (pos > SERVO_MAX) pos = SERVO_MAX;

  return static_cast<uint16_t>(round(pos));
}

int16_t ServoControl::setCoordinatePosition(double x, double y) {
  fprintf(stdout, "X: %f, Y: %f\n", x, y);


  double distanceSquared = x * x + y * y;
  double distance = sqrt(distanceSquared);

  // Check reachability
  if (distance > (L1 + L2) || distance < fabs(L1 - L2)) {
      fprintf(stderr, "Target out of reach.\n");
      return false;
  }

  // Law of cosines
  double cosAngle2 = (distanceSquared - L1*L1 - L2*L2) / (2 * L1 * L2);
  double angle2 = acos(cosAngle2); // Elbow angle (radians)

  double k1 = L1 + L2 * cos(angle2);
  double k2 = L2 * sin(angle2);
  double angle1 = atan2(y, x) - atan2(k2, k1); // Shoulder angle (radians)

  // Convert to degrees
  double theta1Deg = radToDeg(angle1);
  double theta2Deg = radToDeg(angle2);

  // Adjust for your servo setup:
  // 1. Offset by 150º when straight up (north)
  // 2. L2 servo spins in opposite direction
  double servo1Angle = 240 - theta1Deg;
  double servo2Angle = 150 - theta2Deg;

  fprintf(stdout, "Servo 1: %f, Servo 2: %f\n", servo1Angle, servo2Angle);

  uint16_t pos1 = angleToPosition(servo1Angle);
  uint16_t pos2 = angleToPosition(servo2Angle);

  fprintf(stdout, "Servo 1 Position: %d, Servo 2 Position: %d %f %f\n", pos1, pos2, radToDeg(angle1), 180 - radToDeg(angle2));

  setPair(SERVO_MAX - angleToPosition(servo1Angle), angleToPosition(servo2Angle));
  return 0;
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

int16_t ServoControl::raisePen() {
  int ret = setPosition(3, 512);
  if (ret != 0) {
    fprintf(stderr, "Failed to raise pen\n");
    return -1;
  }
  return 0;
}

int16_t ServoControl::dropPen() {
  int ret = setPosition(3, 1023);
  if (ret != 0) {
    fprintf(stderr, "Failed to drop pen\n");
    return -1;
  }
  return 0;
}

int16_t ServoControl::calibratePen() {
  
}