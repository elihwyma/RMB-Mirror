#include <stdio.h>
#include <tensorflow/lite/interpreter.h>

class Interpreter {
public:
    Interpreter(const char *model_path);
private:
    std::unique_ptr<tflite::Interpreter> interpreter;
};