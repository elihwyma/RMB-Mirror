#pragma once

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
#include <uinstd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

// We're using SH1106

#define WIDTH 128
#define HEIGHT 64

class OLEDScreen {
public:
    OLEDScreen(int bus, int address);
    ~OLEDScreen();
private:
    int fileDescriptor = -1;
    int bus;

};
