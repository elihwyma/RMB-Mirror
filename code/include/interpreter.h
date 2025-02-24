#include <stdio.h>
#include <tensorflow/lite/interpreter.h>
#include <opencv2/opencv.hpp>

#define IMAGE_SIZE 256
#define OUTPUT_COUNT 478

typedef struct Landmark {
    float32_t x;
    float32_t y;
    float32_t z;
} Landmark;

class Interpreter {
public:
    Interpreter(const char *model_path);

    void infer(const cv::Mat input, float32_t *confidence, Landmark landmarks[OUTPUT_COUNT]);
private:
    std::unique_ptr<tflite::Interpreter> interpreter;
};