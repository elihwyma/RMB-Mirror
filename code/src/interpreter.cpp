#include <interpreter.h>
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/tools/gen_op_registration.h>
#include <tensorflow/lite/kernels/register.h>
#include <math.h>
#include <iostream>
#include <string>

#define EULER_NUMBER_F 2.71828182846
float32_t sigmoid(float32_t n) {
    return (1 / (1 + powf(EULER_NUMBER_F, -n)));
}

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

void Interpreter::infer(const cv::Mat input, float32_t *confidence, Landmark landmarks[OUTPUT_COUNT]) {
    cv::Mat inputResized;
    cv::Mat rgb;
    cv::Mat floatInput;
    cv::resize(input, inputResized, cv::Size(IMAGE_SIZE, IMAGE_SIZE));
    cv::cvtColor(inputResized, rgb, cv::COLOR_BGR2RGB);
    rgb.convertTo(floatInput, CV_32F);

    float32_t* inputTensor = interpreter->typed_input_tensor<float32_t>(0);

    memcpy(inputTensor, floatInput.data, IMAGE_SIZE * IMAGE_SIZE * 3 * sizeof(float32_t));

    if (interpreter->Invoke() != kTfLiteOk) {
        std::cerr << "Error: Failed to run inference!" << std::endl;
        return;
    }
    fprintf(stdout, "3\n");
    float32_t *confidenceTensor = interpreter->typed_output_tensor<float32_t>(1);
    if (confidenceTensor != nullptr) {
        *confidence = confidenceTensor[0];
        *confidence = sigmoid(*confidence);
    }
    fprintf(stdout, "4 (confidence: %f)\n", *confidence);
    float32_t *outputLandmarks = interpreter->typed_output_tensor<float32_t>(0);
    for (int i = 0; i < OUTPUT_COUNT; i++) {
        landmarks[i].x = outputLandmarks[i * 3];
        landmarks[i].y = outputLandmarks[i * 3 + 1];
        landmarks[i].z = outputLandmarks[i * 3 + 2];
    }
    fprintf(stdout, "5\n");
}
