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

// Implementation of https://github.com/patlevin/face-detection-tflite/blob/17e567dc0b7acc77acb857e7a3b08793062822a3/fdlite/transform.py#L238
std::vector<std::vector<float>> Interpreter::project_landmarks(const std::vector<float> input, const cv::Size tensor_size, const cv::Rect padding) {
    // Reshape the data to be a 2D array in groups of 3
    int cols = 3;
    int rows = input.size() / cols;

    std::vector<std::vector<float>> points(rows, std::vector<float>(cols));
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            points[i][j] = input[i * cols + j];
        }
    }

    int32_t width = tensor_size.width;
    int32_t height = tensor_size.height;

    // Undo padding
    for (int i = 0; i < rows; i++) {
        points[i][0] -= padding.x;
        points[i][1] -= padding.y;
    }
    
    for (int i = 0; i < rows; i++) {
        points[i][0] /= width;
        points[i][1] /= height;
        points[i][2] /= width;
    }

    return points;
}

void resizeWithBorder(const cv::Mat &input, cv::Mat &output, cv::Rect &padding, int32_t targetSize) {
    cv::Mat resized;

    int32_t max = std::max(input.rows, input.cols);
    float scale = float(targetSize) / float(max);

    // Resize the image
    cv::resize(input, resized, cv::Size(), scale, scale);
    int32_t xOffset = 0;
    int32_t yOffset = 0;
    if (resized.rows > resized.cols) {
        xOffset = (targetSize - resized.cols) / 2;
    } else {
        yOffset = (targetSize - resized.rows) / 2;
    }
    padding = cv::Rect(xOffset, yOffset, resized.cols, resized.rows);

    cv::copyMakeBorder(resized, output, yOffset, yOffset, xOffset, xOffset, cv::BORDER_CONSTANT, cv::Scalar(0, 0, 0));
}

void Interpreter::infer(const cv::Mat input, float32_t *confidence, Landmark landmarks[OUTPUT_COUNT]) {
    cv::Mat inputResized;
    // cv::Mat rgb;
    cv::Mat floatInput;
    cv::Rect padding;

    cv::resize(input, inputResized, cv::Size(IMAGE_SIZE, IMAGE_SIZE));
    // resizeWithBorder(input, inputResized, padding, IMAGE_SIZE);

    // cv::cvtColor(inputResized, rgb, cv::COLOR_BGR2RGB);
    inputResized.convertTo(floatInput, CV_32FC3, 1.0 / 255.0);

    float32_t* inputTensor = interpreter->typed_input_tensor<float32_t>(0);
    memcpy(inputTensor, floatInput.data, IMAGE_SIZE * IMAGE_SIZE * 3 * sizeof(float32_t));

    if (interpreter->Invoke() != kTfLiteOk) {
        fprintf(stderr, "Failed to run inference\n");
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
    std::vector<float> outputLandmarksVector(outputLandmarks, outputLandmarks + OUTPUT_COUNT * 3);

    for (uint32_t i = 0; i < OUTPUT_COUNT; i += 3) {
           
        // fprintf(stdout, "Drawing: %f %f\n", landmarks[i].x, landmarks[i].y);
        cv::circle(inputResized, {int(outputLandmarks[i]), int(outputLandmarks[i + 1])}, 2, cv::Scalar(0, 255, 0), -1);
    }
    cv::imshow("Face Landmark Detection", inputResized);
    cv::waitKey(1);

    return;
    
    int32_t max = std::max(input.rows, input.cols);
    float scaleFactor = float(IMAGE_SIZE) / float(max);
    int32_t leftBuffer = float(padding.x);
    int32_t topBuffer = float(padding.y);

    fprintf(stdout, "Scale Factor: %f\n", scaleFactor);

    for (uint32_t i = 0; i < OUTPUT_COUNT; i += 3) {
        float x = outputLandmarksVector[i] - leftBuffer;
        float y = outputLandmarksVector[i + 1] - topBuffer;
        float z = float(outputLandmarksVector[i + 2]);

        float xScaled = x / scaleFactor;
        float yScaled = y / scaleFactor;
        landmarks[i / 3] = {xScaled, yScaled, z};
    }
    fprintf(stdout, "5\n");
}
