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
    cv::Mat frame;
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

        if (!cameraCapture.read(frame)) {
            std::cerr << "Error reading frame." << std::endl;
            break;
        }

        try {
            std::vector<cv::Point2i> landmarks = extractor.Process(frame);
            fprintf(stdout, "Found a face\n");

            // Draw the important details in black dots
            
            const int importantIndexes[] = { 
                185, 40, 39, 37, 0, 267, 269, 270, 409,
                61, 146, 91, 181, 84, 17, 314, 405, 321,
                375, 291, 33, 7, 163, 144, 145, 153, 154, 155, 133,
                246, 161, 160, 159, 158, 157, 173,
                263, 249, 390, 373, 374, 380, 381, 382, 362,
                466, 388, 387, 386, 385, 384, 398,
            };

            std::vector<cv::Point2i> importantPoints;
            for (int i = 0; i < sizeof(importantIndexes) / sizeof(importantIndexes[0]); i++) {
                importantPoints.push_back(landmarks[importantIndexes[i]]);
            }

            // Calculate highest X, lowest X, highest Y, lowestY.
            double lowestX = importantPoints[0].x;
            double highestX = importantPoints[0].x;
            double lowestY = importantPoints[0].y;
            double highestY = importantPoints[0].y;

            for (size_t i = 1; i < importantPoints.size(); i++) {
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

            std::vector<cv::Point2i> scaledPoints;
            for (size_t i = 0; i < importantPoints.size(); i++) {
                scaledPoints.push_back(cv::Point2i(
                    (importantPoints[i].x - xOffset) * largestScale,
                    ((importantPoints[i].y - yOffset) * largestScale) + 100
                ));
            }

            for (size_t i = 0; i < scaledPoints.size(); i++) {
                fprintf(stdout, "Going to %d, %d\n", scaledPoints[i].x, scaledPoints[i].y);
                control.interpolate(scaledPoints[i].x, scaledPoints[i].y);
            }
            stepper.step(300);
            
        } catch (const std::invalid_argument& e) {
            fprintf(stderr, "No Face!\n");
            continue;
        }
    }

    cameraCapture.release();
    
    return 0;
}
