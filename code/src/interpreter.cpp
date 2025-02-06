#include <interpreter.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/tools/gen_op_registration.h>
#include <tensorflow/lite/kernels/register.h>

Interpreter::Interpreter(const char *model_path) {
    std::unique_ptr<tflite::FlatBufferModel> model = tflite::FlatBufferModel::BuildFromFile(model_path);
    if (model == nullptr) {
        fprintf(stderr, "Failed to build model from file %s\n", model_path);
        exit(1);
    }
    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder(*model.get(), resolver)(&this->interpreter);
    if (this->interpreter == nullptr) {
        fprintf(stderr, "Failed to build interpreter\n");
        exit(1);
    }
    if (this->interpreter->AllocateTensors() != kTfLiteOk) {
        fprintf(stderr, "Failed to allocate tensors\n");
        exit(1);
    }
}
