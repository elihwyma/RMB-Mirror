#if GPIOD

#include <stepper_control.h>

StepperControl::StepperControl() {
    this->chip = gpiod_chip_open_by_name("gpiochip0");
    if (!this->chip) {
        fprintf(stderr, "Failed to open GPIO chip\n");
        exit(1);
    }

    this->stepLine = gpiod_chip_get_line(this->chip, STEP_PIN);
    this->dirLine = gpiod_chip_get_line(this->chip, DIR_PIN);

    if (!this->stepLine || !this->dirLine) {
        fprintf(stderr, "Failed to get GPIO lines\n");
        gpiod_chip_close(this->chip);
        exit(1);
    }

    if (gpiod_line_request_output(this->stepLine, CONSUMER, 0) < 0 ||
        gpiod_line_request_output(this->dirLine, CONSUMER, 0) < 0) {
        fprintf(stderr, "Failed to request GPIO lines as outputs\n");
        gpiod_chip_close(this->chip);
        exit(1);
    }
}

StepperControl::~StepperControl() {
    gpiod_line_release(this->stepLine);
    gpiod_line_release(this->dirLine);
    gpiod_chip_close(this->chip);
}

#endif