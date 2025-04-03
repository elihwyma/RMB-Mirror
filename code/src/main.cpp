#include <cstdio>
#include <stdexcept>
#include <stdio.h>
#include <servo_control.h>
#include <iostream>
#include <runtime_types.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <stepper_control.h>
#include <landmark_extractor.h>

int main(int argc, char* argv[]) {
    printf("Hello, World!\n");

    ServoControl control;
    #if GPIOD
    StepperControl stepper;
    #endif

    std::string landmarkModelPath = "resources/face_landmarks.tflite";
    std::string detectionModelPath = "resources/face_detection_short_range.tflite";
    fprintf(stdout, "1\n");
    LandmarkExtractor extractor(detectionModelPath, landmarkModelPath);
    fprintf(stdout, "2\n");
    int cameraID = 0;

    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            try {
                cameraID = atoi(argv[i + 1]);
            } catch(...) {
                // 
            }
        }
    }

    cv::VideoCapture cameraCapture(cameraID);
   
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

        try {
            std::vector<cv::Point2i> landmarks = extractor.Process(frame);
            for (const auto& landmark : landmarks) {
                cv::circle(frame, landmark, 2, cv::Scalar(0, 255, 0), -1);
            }
        } catch (const std::invalid_argument& e) {
            fprintf(stderr, "No Face!\n");
        }
        
        cv::imshow("Face Landmark Detection", frame);
        if (cv::waitKey(1) == 'q') {
            break;
        }        
    }

    cameraCapture.release();
    cv::destroyAllWindows();

    return 0;
}
