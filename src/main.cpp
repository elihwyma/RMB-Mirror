#include <cstdint>
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

    StepperControl stepper;
    stepper.setServoPower(true);

    ServoControl control;
    


    if (argc >= 2 && strcmp(argv[1], "debug") == 0) {
        stepper.setServoPower(true);
        stepper.step(100);
        stepper.setServoPower(false);

        for (;;) {
            std::string input;
            std::cout << "Enter X and Y coordinates (or 'q' to quit, alternatively 's' to step): ";
            std::getline(std::cin, input);
    
            if (input == "q") {
                break;
            }
            if (input == "s") {
                stepper.step(100);
                continue;
            }
            if (input == "ss") {
                stepper.step(10000);
                continue;
            }
            std::istringstream iss(input);
            double x, y;
            if (!(iss >> x >> y)) {
                std::cerr << "Invalid input. Please enter two numbers." << std::endl;
                continue;
            }
    
            // Call the Interpolate function
            control.interpolate(x, y);
        }
    }

    control.calibratePen();

    std::string landmarkModelPath = "face_landmarks.tflite";
    std::string detectionModelPath = "face_detection_short_range.tflite";

    if (access(landmarkModelPath.c_str(), F_OK)) {
        landmarkModelPath = "resources/" + landmarkModelPath;
    }
    if (access(detectionModelPath.c_str(), F_OK)) {
        detectionModelPath = "resources/" + detectionModelPath;
    }

    LandmarkExtractor extractor(detectionModelPath, landmarkModelPath);
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

    // Main Program Loop
    bool pressed = false;
    for (;;) {
        stepper.activateLED();
        bool _pressed = stepper.isButtonPressed();

        if (_pressed && pressed) {
            // Button is still pressed
            usleep(100000);
            continue;
        }
        if (!_pressed) {
            pressed = false;
            continue;
        }
        pressed = true;

        // Deactivate Button to signify that work is happening
        stepper.deactivateLED();

        cv::Mat frame;

        if (!cameraCapture.read(frame)) {
            std::cerr << "Error reading frame." << std::endl;
            break;
        }

        try {
            std::vector<cv::Point2i> landmarks = extractor.Process(frame);
            fprintf(stdout, "Found a face\n");

            const uint16_t leftEye[] = {
                398, 384, 385, 386, 387, 388, 466, 263, 249, 390, 373, 374, 380, 381 
            };
            const uint16_t rightEye[] = {
                173, 157, 158, 159, 160, 161, 246, 7, 163, 144, 145, 153, 154
            };
            const uint16_t mouth[] = {
                14, 317, 402, 318, 324, 292, 308, 407, 415, 272, 310, 271, 311, 268, 12, 38, 81, 191, 80, 183, 62, 95, 88, 178, 87
            };
            const uint16_t leftEyeSize = sizeof(leftEye) / sizeof(leftEye[0]);
            const uint16_t rightEyeSize = sizeof(rightEye) / sizeof(rightEye[0]);
            const uint16_t mouthSize = sizeof(mouth) / sizeof(mouth[0]);
            const uint16_t importantPointsSize = leftEyeSize + rightEyeSize + mouthSize;
            
            cv::Point2i importantPoints[importantPointsSize];
            cv::Point2i scaledPoints[importantPointsSize];

            for (size_t i = 0; i < leftEyeSize; i++) {
                importantPoints[i] = landmarks[leftEye[i]];
            }
            for (size_t i = 0; i < rightEyeSize; i++) {
                importantPoints[i + leftEyeSize] = landmarks[rightEye[i]];
            }
            for (size_t i = 0; i < mouthSize; i++) {
                importantPoints[i + leftEyeSize + rightEyeSize] = landmarks[mouth[i]];
            }
            
            // Calculate highest X, lowest X, highest Y, lowestY.
            double lowestX = importantPoints[0].x;
            double highestX = importantPoints[0].x;
            double lowestY = importantPoints[0].y;
            double highestY = importantPoints[0].y;

            for (uint16_t i = 1; i < importantPointsSize; i++) {
                if (importantPoints[i].x < lowestX) {
                    lowestX = importantPoints[i].x;
                }
                if (importantPoints[i].x > highestX) {
                    highestX = importantPoints[i].x;
                }
                if (importantPoints[i].y < lowestY) {
                    lowestY = importantPoints[i].y;
                }
                if (importantPoints[i].y > highestY) {
                    highestY = importantPoints[i].y;
                }
            }

            double xOffset = lowestX;
            double yOffset = lowestY;
            double xWidth = highestX - lowestX;
            double yHeight = highestY - lowestY;

            /*
            For debug purposes we're going to say our usable area is [
                [-25, 100,],
                [ 235, 235 ]
            ]
            */

            #define LOWEST_X -25
            #define LOWEST_Y 125
            #define HIGHEST_X 235
            #define HIGHEST_Y 235

            double xScale = (HIGHEST_X - std::abs(LOWEST_X)) / xWidth;
            double yScale = (HIGHEST_Y - LOWEST_Y) / yHeight;

            double largestScale = std::max(xScale, yScale);

            for (size_t i = 0; i < importantPointsSize; i++) {
                scaledPoints[i] = cv::Point2i(
                    (importantPoints[i].x - xOffset) * largestScale,
                    ((importantPoints[i].y - yOffset) * largestScale) + 100
                );
            }
            
            control.raisePen();
            control.interpolate(scaledPoints[0].x, scaledPoints[0].y);
            control.dropPen();

            for (uint16_t i = 1; i < leftEyeSize; i++) {
                fprintf(stdout, "Going to %d, %d\n", scaledPoints[i].x, scaledPoints[i].y);
                control.interpolate(scaledPoints[i].x, scaledPoints[i].y);
            }

            control.raisePen();
            control.interpolate(scaledPoints[leftEyeSize].x, scaledPoints[leftEyeSize].y);
            control.dropPen();

            for (uint16_t i = leftEyeSize + 1; i < leftEyeSize + rightEyeSize; i++) {
                fprintf(stdout, "Going to %d, %d\n", scaledPoints[i].x, scaledPoints[i].y);
                control.interpolate(scaledPoints[i].x, scaledPoints[i].y);
            }

            control.raisePen();
            control.interpolate(scaledPoints[leftEyeSize + rightEyeSize].x, scaledPoints[leftEyeSize + rightEyeSize].y);
            control.dropPen();

            for (uint16_t i = leftEyeSize + rightEyeSize + 1; i < leftEyeSize + rightEyeSize + mouthSize; i++) {
                fprintf(stdout, "Going to %d, %d\n", scaledPoints[i].x, scaledPoints[i].y);
                control.interpolate(scaledPoints[i].x, scaledPoints[i].y);
            }
            control.raisePen();
                
            stepper.step(300);
        } catch (const std::invalid_argument& e) {
            fprintf(stderr, "No Face!\n");
            continue;
        }
    }

    cameraCapture.release();
    
    return 0;
}
