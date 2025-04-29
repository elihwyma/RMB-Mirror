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
#include <unistd.h>

int main(int argc, char* argv[]) {
    printf("Hello, World!\n");

    ServoControl control;
    #if GPIOD
    // StepperControl stepper;
    #endif

    int16_t ret = 0;
    for (;;) {
        for (int i = -150; i < 200; i += 15) {
            ret = control.setCoordinatePosition(i, 100);
            if (ret != 0) {
                fprintf(stderr, "Failed to set position\n");
                return -1;
            }
        }
    }
    exit(0);

    std::string landmarkModelPath = "face_landmarks.tflite";
    std::string detectionModelPath = "face_detection_short_range.tflite";

    if (access(landmarkModelPath.c_str(), F_OK)) {
        landmarkModelPath = "resources/" + landmarkModelPath;
    }
    if (access(detectionModelPath.c_str(), F_OK)) {
        detectionModelPath = "resources/" + detectionModelPath;
    }

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

            // Create a new blank image to draw our contours
            // We want a white background with the important detaits in black dots
            frame = cv::Mat::zeros(frame.size(), frame.type());
            // Draw a white background
            frame.setTo(cv::Scalar(255, 255, 255));
            // Draw the important details in black dots
            /*
            const int importantIndexes[] = { 
                185, 40, 39, 37, 0, 267, 269, 270, 409,
                61, 146, 91, 181, 84, 17, 314, 405, 321,
                375, 291, 33, 7, 163, 144, 145, 153, 154, 155, 133,
                246, 161, 160, 159, 158, 157, 173,
                263, 249, 390, 373, 374, 380, 381, 382, 362,
                466, 388, 387, 386, 385, 384, 398,
            };*/
            for (int i = 0; i < landmarks.size(); i++) {
                cv::circle(frame, landmarks[i], 2, cv::Scalar(0, 0, 0), -1);
            }
        } catch (const std::invalid_argument& e) {
            fprintf(stderr, "No Face!\n");
        }
        #if GPIOD
        #else
        cv::imshow("Face Landmark Detection", frame);
        if (cv::waitKey(1) == 'q') {
            break;
        }
        #endif        
    }

    cameraCapture.release();
    
    #if GPIOD
    #else
    cv::destroyAllWindows();
    #endif

    return 0;
}
