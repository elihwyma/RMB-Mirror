#pragma once

#include <stdio.h>
#include <string>
#include <cpr/cpr.h>

class Logging {
public:
    Logging() =  default;
    ~Logging() = default;

    void log(const std::string &error, const double servoOneTemp, double servoTwoTemp, double penServoTemp);
};