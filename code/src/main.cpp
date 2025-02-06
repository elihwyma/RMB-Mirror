#include <stdio.h>
#include <interpreter.h>
#include <opencv2/opencv.hpp>

Interpreter interpreter("resources/face_landmark.tflite");
cv::VideoCapture cameraCapture(0);

int main() {
    printf("Hello, World!\n");

    return 0;
}