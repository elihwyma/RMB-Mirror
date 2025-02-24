#include <stdio.h>
#include <interpreter.h>
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>

int main(int argc, char* argv[]) {
    printf("Hello, World!\n");

    std::string modelPath = "resources/face_landmark.tflite";
    int cameraID = 0;

    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-m") == 0) {

            modelPath = argv[i + 1];
        }
        if (strcmp(argv[i], "-v") == 0) {
            try {
                cameraID = atoi(argv[i + 1]);
            } catch(...) {
                // 
            }
        }
    }

    cv::VideoCapture cameraCapture(cameraID);
    Interpreter interpreter(modelPath.c_str());

    if (!cameraCapture.isOpened()) {
        std::cerr << "Error opening camera." << std::endl;
        return -1;
    }
    return 0;
}