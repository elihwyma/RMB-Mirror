#include <cstdint>
#include <cstdio>
#include <servo_control.h>
#include <filesystem>
#include <sys/types.h>
#include <cmath>
#include <unistd.h>
#include <csignal>

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
#define L2 165.0  // mm
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

std::string ServoControl::errorDescription(int error) {
    return "";
}

static ServoControl* instance;

ServoControl::ServoControl(StepperControl &stepper) : stepper(stepper) {
    std::string port_name;
    getPortName(&port_name);

    PortHandler *portHandler = PortHandler::getPortHandler(port_name.c_str());
    PacketHandler *packetHandler = PacketHandler::getPacketHandler(PROTOCOL_VERSION);

    this->portHandler = portHandler;
    this->packetHandler = packetHandler;

    if (openPort() != 0) {
        fprintf(stderr, "Failed to open port\n");
        std::raise(SIGTERM);
    }

    int16_t error = 0;
    error = setPositionMode(1);
    if (error != 0) {
        fprintf(stderr, "Failed to set position mode on servo 1\n");
        std::raise(SIGTERM);
    }
    error = setPositionMode(2);
    if (error != 0) {
        fprintf(stderr, "Failed to set position mode on servo 2\n");
        std::raise(SIGTERM);
    }
    fprintf(stdout, "Homing Servos...\n");

    setCoordinatePosition(0, 250);

    error = setWheelMode(3);
    if (error != 0) {
        fprintf(stderr, "Failed to set wheel mode on servo 3\n");
        std::raise(SIGTERM);
    }
    
    setWheelSpeed(3, 0, 0);
    // Set 50% to L1 and L2
    setWheelSpeed(2, 0, 512);
    setWheelSpeed(1, 0, 512);

    instance = this;

    std::signal(SIGINT, signalHandler);   // Ctrl+C
    std::signal(SIGTERM, signalHandler);  // Termination
    std::signal(SIGABRT, signalHandler);  // Abnormal termination
    std::signal(SIGSEGV, signalHandler);  // Segmentation fault
    std::signal(SIGFPE, signalHandler);   // Floating-point exception
    std::signal(SIGILL, signalHandler);   // Floating-point exception
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
        std::exit(SIGABRT);
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

  if (this->currentX == x && this->currentY == y) {
    fprintf(stdout, "Already at target position\n");
    return 0;
  }

  this->currentX = x;
  this->currentY = y;

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

int16_t ServoControl::interpolate(double targetx, double targety) {
  double diff_x = targetx - this->currentX;
  double diff_y = targety - this->currentY;
  size_t steps = std::max(std::abs(diff_x), std::abs(diff_y)) / 10;

  double stepX = diff_x / steps;
  double stepY = diff_y / steps;

  fprintf(stdout, "Interpolating to %f, %f %f %f\n", targetx, targety, stepX, stepY);

  for (size_t i = 0; i < steps; ++i) {
    double x = this->currentX + stepX;
    double y = this->currentY + stepY;
    setCoordinatePosition(x, y);
  }
  // Ensure we didn't fall fictim to poor floating point handling
  setCoordinatePosition(targetx, targety);
  return 0;
}

int16_t ServoControl::raisePen() {
  if (!this->stepper.isPenTouching()) {
    fprintf(stdout, "Pen already raised\n");
    return 0;
  }
  int ret = setWheelSpeed(3, 0, 1023);
  if (ret != 0) {
    fprintf(stderr, "Failed to raise pen\n");
    return -1;
  }
  while (this->stepper.isPenTouching()) {
    usleep(100);
  }
  ret = setWheelSpeed(3, 1, 0);
  if (ret != 0) {
    fprintf(stderr, "Failed to raise pen\n");
    return -1;
  }
  return ret;
}

int16_t ServoControl::dropPen() {
  if (this->stepper.isPenTouching()) {
    fprintf(stdout, "Pen already dropped\n");
    return 0;
  }
  int ret = setWheelSpeed(3, 1, 1023);  
  if (ret) {
    fprintf(stderr, "Failed to drop pen\n");
    return -1;
  }
  while (!this->stepper.isPenTouching()) {
    usleep(100);
  }
  ret = setWheelSpeed(3, 0, 0);
  if (ret) {
    fprintf(stderr, "Failed to drop pen\n");
    return -1;
  }
  return ret;
}

int16_t ServoControl::calibratePen() {
  return raisePen();
}

void ServoControl::signalHandler(int signum) {
    fprintf(stdout, "Caught signal %d\n", signum);
    if (instance) {
        instance->setWheelSpeed(3, 0, 0);
        instance->closePort();
    }
    std::exit(signum);
}