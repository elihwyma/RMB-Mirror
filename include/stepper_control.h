#if GPIOD

#include <gpiod.h>
#include <unistd.h>
#include <sys/types.h>

#define CONSUMER "rate-my-bot"
#define DIR_PIN 24
#define STEP_PIN 23
#define SERVO_POWER 20

class StepperControl {
public:
    StepperControl();
    ~StepperControl();

    void setServoPower(bool power);
    void step(int64_t);
private:
    struct gpiod_chip *chip;
    struct gpiod_line *stepLine;
    struct gpiod_line *dirLine;
    struct gpiod_line *servoPowerLine;
};

#endif