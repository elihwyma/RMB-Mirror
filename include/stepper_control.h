#if GPIOD

#include <gpiod.h>
#include <unistd.h>
#include <sys/types.h>

#define CONSUMER "rate-my-bot"
#define DIR_PIN 24
#define STEP_PIN 23
#define SERVO_POWER 20
#define BUTTON_LED 16
#define BUTTON_INPUT 21

class StepperControl {
public:
    StepperControl();
    ~StepperControl();

    void setServoPower(bool power);
    void step(int64_t);

    void activateLED();
    void deactivateLED();

    bool isButtonPressed();
private:
    struct gpiod_chip *chip;
    struct gpiod_line *stepLine;
    struct gpiod_line *dirLine;
    struct gpiod_line *servoPowerLine;
    struct gpiod_line *buttonLedLine;
    struct gpiod_line *buttonInputLine;

    bool lightActive = true;
};

#endif