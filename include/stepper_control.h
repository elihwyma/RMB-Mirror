#if GPIOD

#include <gpiod.h>
#include <unistd.h>

#define CONSUMER "rate-my-bot"
#define DIR_PIN 23
#define STEP_PIN 24

class StepperControl {
public:
    StepperControl();
    ~StepperControl();
private:
    struct gpiod_chip *chip;
    struct gpiod_line *stepLine;
    struct gpiod_line *dirLine;
};

#endif