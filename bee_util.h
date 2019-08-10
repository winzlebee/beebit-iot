#pragma once

#include <string>
#include <iostream>

namespace beebit {

// Contains default configuration variables
struct Configuration {
    // Location of the trained model
    std::string modelLocation = "dnn/yolov3.weights";
    // Location of the config for the neural network
    std::string configLocation = "dnn/config.cfg";
    // Whether to use the raspberry pi camera
    bool usePiCam = false;
    // Number of frames to skip during detection
    int skipFrames = 5;
    // Minimum probability to filter weak detections
    float confidence = 0.4;
};

Configuration *loadConfig();
void writeConfig(const Configuration &conf);

template<typename T>
void log(const T &text) {
    std::cout << "BeeBit: " << text << std::endl;
}

}