#pragma once

#include <gpiod.h>
#include <unistd.h>
#include <sys/types.h>
#include <csignal>

#define CONSUMER "rate-my-bot"
#define DIR_PIN 24
#define STEP_PIN 23
#define SERVO_POWER 20
#define BUTTON_LED 16
#define BUTTON_INPUT 21
#define PEN_TOUCHING 27

class StepperControl {
public:
    StepperControl();
    ~StepperControl();

    void setServoPower(bool power);
    void step(int64_t);

    void activateLED();
    void deactivateLED();

    bool isButtonPressed();
    bool isPenTouching();
private:
    struct gpiod_chip *chip;
    struct gpiod_line *stepLine;
    struct gpiod_line *dirLine;
    struct gpiod_line *servoPowerLine;
    struct gpiod_line *buttonLedLine;
    struct gpiod_line *buttonInputLine;
    struct gpiod_line *penTouchingLine;

    bool lightActive = false;
};
