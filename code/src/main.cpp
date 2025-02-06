#include <stdio.h>
#include <interpreter.h>

int main() {
    printf("Hello, World!\n");

    Interpreter tmp("resources/face_landmark.tflite");

    return 0;
}