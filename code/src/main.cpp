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

    cv::Mat frame;
    while (1) {
        if (!cameraCapture.read(frame)) {
            std::cerr << "Error reading frame." << std::endl;
            break;
        }

        float32_t confidence = 0;
        Landmark landmarks[OUTPUT_COUNT];

        interpreter.infer(frame, &confidence, landmarks);
        fprintf(stdout, "Confidence: %f\n", confidence);

        for (uint32_t i = 0; i < OUTPUT_COUNT; i++) {
            fprintf(stdout, "Drawing: %f %f\n", landmarks[i].x, landmarks[i].y);
            cv::circle(frame, {int(landmarks[i].x), int(landmarks[i].y)}, 5, cv::Scalar(0, 255, 0), -1);
        }

        cv::imshow("Webcam", frame);
        if (cv::waitKey(1) == 'q') {
            break;
        }        
    }

    cameraCapture.release();
    cv::destroyAllWindows();

    return 0;
}