#include <cstdio>
#include <cstdlib>
#include <stepper_control.h>
#include <unistd.h>

StepperControl::StepperControl() {
    this->chip = gpiod_chip_open_by_name("gpiochip0");
    if (!this->chip) {
        fprintf(stderr, "Failed to open GPIO chip\n");
        exit(1);
    }

    this->stepLine = gpiod_chip_get_line(this->chip, STEP_PIN);
    this->dirLine = gpiod_chip_get_line(this->chip, DIR_PIN);
    this->servoPowerLine = gpiod_chip_get_line(this->chip, SERVO_POWER);
    this->buttonLedLine = gpiod_chip_get_line(this->chip, BUTTON_LED);
    this->buttonInputLine = gpiod_chip_get_line(this->chip, BUTTON_INPUT);
    this->penTouchingLine = gpiod_chip_get_line(this->chip, PEN_TOUCHING);

    if (!this->stepLine || !this->dirLine || !this->servoPowerLine || !this->buttonLedLine || !this->buttonInputLine) {
        fprintf(stderr, "Failed to get GPIO lines\n");
        gpiod_chip_close(this->chip);
        exit(1);
    }

    if (gpiod_line_request_output(this->stepLine, CONSUMER, 0) < 0 ||
        gpiod_line_request_output(this->dirLine, CONSUMER, 0) < 0 || 
        gpiod_line_request_output(this->servoPowerLine, CONSUMER, 0) < 0 ||
        gpiod_line_request_output(this->buttonLedLine, CONSUMER, 0) < 0 ||
        gpiod_line_request_input(this->buttonInputLine, CONSUMER) < 0 ||
        gpiod_line_request_input(this->penTouchingLine, CONSUMER) < 0) {
        fprintf(stderr, "Failed to request GPIO lines as outputs\n");
        gpiod_chip_close(this->chip);
        exit(1);
    }

    gpiod_line_set_value(this->stepLine, 0);
    gpiod_line_set_value(this->dirLine, 0);
    gpiod_line_set_value(this->servoPowerLine, 0);
    gpiod_line_set_value(this->buttonLedLine, this->lightActive ? 1 : 0);
}

StepperControl::~StepperControl() {
    gpiod_line_release(this->stepLine);
    gpiod_line_release(this->dirLine);
    gpiod_line_release(this->servoPowerLine);
    gpiod_chip_close(this->chip);
}

void StepperControl::setServoPower(bool power) {
    int error = gpiod_line_set_value(this->servoPowerLine, power ? 1 : 0);
    if (error) {
        fprintf(stderr, "Failed to set servo power %d\n", error);
    } else {
        fprintf(stdout, "Servo power %s\n", power ? "ON" : "OFF");
    }
}

void StepperControl::step(int64_t steps) {
    bool dir = steps > 0;
    gpiod_line_set_value(this->dirLine, dir);

    uint16_t abs_steps = abs(steps);
    for (uint16_t i = 0; i < abs_steps; i++) {
        gpiod_line_set_value(this->stepLine, 1);
        usleep(3000); // Adjust the delay as needed
        gpiod_line_set_value(this->stepLine, 0);
        usleep(3000); // Adjust the delay as needed
    }
}

void StepperControl::activateLED() {
    if (this->lightActive) {
        return;
    }
    this->lightActive = true;
    gpiod_line_set_value(this->buttonLedLine, 1);
}

void StepperControl::deactivateLED() {
    if (!this->lightActive) {
        return;
    }
    this->lightActive = false;
    gpiod_line_set_value(this->buttonLedLine, 0);
}

bool StepperControl::isButtonPressed() {
    return gpiod_line_get_value(this->buttonInputLine) == 0;
}

bool StepperControl::isPenTouching() {
    return gpiod_line_get_value(this->penTouchingLine) == 1;
}