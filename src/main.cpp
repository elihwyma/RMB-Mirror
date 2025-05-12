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
#include <vector>

#define LOWEST_X -80
#define LOWEST_Y 150
#define HIGHEST_X 200
#define HIGHEST_Y 250

#define MAX_WIDTH (HIGHEST_X - LOWEST_X)
#define MAX_HEIGHT (HIGHEST_Y - LOWEST_Y)

int main(int argc, char* argv[]) {
    printf("Hello, World!\n");

    StepperControl stepper;
    stepper.setServoPower(true);

    ServoControl control(stepper);
    
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

    printf("Camera ID: %d\n", cameraID);
    cv::VideoCapture cameraCapture(cameraID);

    if (!cameraCapture.isOpened()) {
        std::cerr << "Error opening camera." << std::endl;
        return -1;
    }
    printf("Camera opened successfully.\n");
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

            const std::vector<std::vector<uint16_t>> landmarkIndexes = {
                { // Left Eye Brow
                    70, 63, 105, 66, 107, 55, 65, 52, 53
                },
                { // Right Eye Brow
                    300, 293, 334, 296, 336, 285, 295, 282, 283
                },
                { // Right Eye
                    414, 286, 258, 257, 259, 260, 467, 359, 381, 380, 374, 373, 390, 249
                },
                { // Left Eye
                    190, 56, 28, 27, 29, 30, 247, 33, 7, 163, 144, 145, 153, 173
                },
                { // Nose
                    193, 188, 174, 236, 131, 115, 79, 20, 354, 459, 438, 344, 360, 456, 399, 412, 417
                },
                { // Outline Mouth
                    61, 185, 40, 39, 37, 0, 267, 269, 270, 409, 291, 375, 321, 405, 314, 17, 84, 181, 91, 146
                },
                { // Inside Mouth
                    14, 317, 402, 318, 324, 292, 308, 407, 415, 272, 310, 271, 311, 268, 12, 38, 81, 191, 80, 183, 62, 95, 88, 178, 87
                },
                { // Face Outline
                    10, 338, 297, 332, 284, 251, 389, 356, 454, 323, 361, 288, 397, 365, 379, 378, 400, 377,
                    152, 148, 176, 149, 150, 136, 172, 58, 132, 93, 234, 127, 162, 21, 54, 103, 67, 109
                }
            };

            std::vector<std::vector<cv::Point2i>> importantPoints(landmarkIndexes.size());
            std::vector<std::vector<cv::Point2i>> scaledPoints(landmarkIndexes.size());

            for (size_t i = 0; i < landmarkIndexes.size(); i++) {
                importantPoints[i] = std::vector<cv::Point2i>(landmarkIndexes[i].size());
                scaledPoints[i] = std::vector<cv::Point2i>(landmarkIndexes[i].size());
                for (size_t j = 0; j < landmarkIndexes[i].size(); j++) {
                    importantPoints[i][j] = landmarks[landmarkIndexes[i][j]];
                }
            }
            
            // Calculate highest X, lowest X, highest Y, lowestY.
            double lowestX = importantPoints[0][0].x;
            double highestX = importantPoints[0][0].x;
            double lowestY = importantPoints[0][0].y;
            double highestY = importantPoints[0][0].y;

            for (size_t i = 0; i < landmarkIndexes.size(); i++) {
                for (size_t j = 0; j < landmarkIndexes[i].size(); j++) {
                    if (importantPoints[i][j].x < lowestX) {
                        lowestX = importantPoints[i][j].x;
                    }
                    if (importantPoints[i][j].x > highestX) {
                        highestX = importantPoints[i][j].x;
                    }
                    if (importantPoints[i][j].y < lowestY) {
                        lowestY = importantPoints[i][j].y;
                    }
                    if (importantPoints[i][j].y > highestY) {
                        highestY = importantPoints[i][j].y;
                    }
                }
            }

            double xOffset = lowestX;
            double yOffset = lowestY;
            double xWidth = highestX - lowestX;
            double yHeight = highestY - lowestY;

            // Output Mat is to help debug
            cv::Mat outputMat(HIGHEST_Y - LOWEST_Y,
                 HIGHEST_X - LOWEST_X,
                  CV_8UC3, cv::Scalar(0, 0, 0));

            double xScale = MAX_WIDTH / xWidth;
            double yScale = MAX_HEIGHT / yHeight;

            double largestScale = std::max(xScale, yScale);

            for (size_t i = 0; i < landmarkIndexes.size(); i++) {
                for (size_t j = 0; j < landmarkIndexes[i].size(); j++) {
                    cv::Point2i outPoint = cv::Point2i(
                        (importantPoints[i][j].x - xOffset) / largestScale,
                        ((importantPoints[i][j].y - yOffset) / largestScale)
                    );
                    cv::circle(outputMat, outPoint, 2, cv::Scalar(255, 0, 0), -1);
                    outPoint.x += LOWEST_X;
                    outPoint.y += LOWEST_Y;
                    scaledPoints[i][j] = outPoint;
                }
            }
            // Write the debug image to disk as a png
            cv::imwrite("output.png", outputMat);
            
            for (size_t i = 0; i < landmarkIndexes.size(); i++) {
                control.raisePen();
                control.setCoordinatePosition(scaledPoints[i][0].x, scaledPoints[i][0].y);
                control.dropPen();
                for (size_t j = 1; j < landmarkIndexes[i].size(); j++) {
                    control.interpolate(scaledPoints[i][j].x, scaledPoints[i][j].y);
                }
                fprintf(stdout, "Going back to start %zu\n", i);    
                control.interpolate(scaledPoints[i][0].x, scaledPoints[i][0].y);
            }
            control.raisePen();
            stepper.step(600);
            control.calibratePen();
        } catch (const std::invalid_argument& e) {
            fprintf(stderr, "No Face!\n");
            continue;
        }
    }

    cameraCapture.release();
    
    return 0;
}
