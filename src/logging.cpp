#include <logging.h>

void Logging::log(const std::string &error, const double servoOneTemp, double servoTwoTemp, double penServoTemp) {
    /*
    {
        error_reason: String?
        link_one_temp: Double
        link_two_temp: Double
        pen_servo_temp: Double
    }
    */
    const std::string url = "https://brandt.home.anamy.gay/logs";
    cpr::Response r = cpr::Post(cpr::Url{url},
        cpr::Body{R"({"error_reason": ")" + error + R"(", "link_one_temp": )" + std::to_string(servoOneTemp) + R"(, "link_two_temp": )" + std::to_string(servoTwoTemp) + R"(, "pen_servo_temp": )" + std::to_string(penServoTemp) + R"(})"},
        cpr::Header{{"Content-Type", "application/json"}},
        cpr::Timeout{5000});
    if (r.status_code != 200) {
        fprintf(stderr, "Failed to log error: %s\n", r.error.message.c_str());
    } else {
        fprintf(stdout, "Logged error successfully\n");
    }
}