#include <stdio.h>
#include <interpreter.h>
#include <servo_control.h>
#include <iostream>
#include <runtime_types.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <gpiod.h>

StepPin = 24;
DirPin = 23;
Dir = 1; // 1 = clockwise, 0 = counter clockwise
const char* chipname = "gpiochip0";

int main(int argc, char* argv[]) {
    printf("Hello, World!\n");

    struct gpiod_chip* chip = gpiod_chip_open_by_name(chipname);
    if (!chip) {
        cerr << "Failed to open GPIO chip\n";
        return -1;
    }

    struct gpiod_line* stepLine = gpiod_chip_get_line(chip, StepPin);
    struct gpiod_line* dirLine = gpiod_chip_get_line(chip, DirPin);

    if (!stepLine || !dirLine) {
        cerr << "Failed to get GPIO lines\n";
        gpiod_chip_close(chip);
        return -1;
    }

    if (gpiod_line_request_output(stepLine, "stepper-motor", 0) < 0 ||
        gpiod_line_request_output(dirLine, "stepper-motor", 0) < 0) {
        cerr << "Failed to request GPIO lines as outputs\n";
        gpiod_chip_close(chip);
        return -1;
    }

    for (int i <= 100, i++) {
        Step(Dir):
    }

    ServoControl control;
    control.setPosition(1, 700);
    control.setPosition(2, 700);

    return 0;

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
        continue;

        for (uint32_t i = 0; i < OUTPUT_COUNT; i++) {
           
            // fprintf(stdout, "Drawing: %f %f\n", landmarks[i].x, landmarks[i].y);
            cv::circle(frame, {int(landmarks[i].x), int(landmarks[i].y)}, 5, cv::Scalar(0, 255, 0), -1);
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

void Step(struct gpiod_line* stepLine) {
    gpiod_line_set_value(stepLine, 1);  // Send a step pulse (HIGH)
    this_thread::sleep_for(chrono::milliseconds(1));  // Wait for the motor to step
    gpiod_line_set_value(stepLine, 0);  // Set STEP pin LOW
    this_thread::sleep_for(chrono::milliseconds(1));  // Wait before the next pulse
}