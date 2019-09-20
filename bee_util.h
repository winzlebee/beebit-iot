#pragma once

#include <string>
#include <iostream>

#include "util/types.h"

namespace beebit {

/**
 * @brief Read a generalized configuration file into a map of configurations
 * 
 * @param location 
 * @return ConfigMap 
 */
ConfigMap readConfiguration(const std::string &location);
void writeConfiguration(std::ostream &out, const ConfigMap &newMap);

// Contains default configuration variables
struct TrackerConfiguration {
    // Location of the trained model
    std::string modelLocation = "dnn/yolov3.weights";
    // Location of the config for the neural network
    std::string configLocation = "dnn/config.cfg";
    // Whether to use the raspberry pi camera
    bool usePiCam = false;
    // Number of frames to skip during detection
    int skipFrames = 5;
    // Minimum probability to filter weak detections
    float confidence = 0.2;
    // Size of the captured image
    int imageWidth = 320;
    int imageHeight = 240;
    // Quality of the neural net detection used
    int neuralNetQuality = 416;
    // Whether to attempt to use OpenCL for computation
    bool useOpenCL = true;
    // Whether to use KCF or the higher quality CSRT
    bool useCSRT = false;
    // Number of frames we haven't detected a person before we consider them missing
    int maxDisappeared = 50;
    // Maximum distance the tracker will consider consolidating points. Should generally be width / n where n is the average people
    // expected in the frame.
    int searchDistance = 50;
    // Distance from the line where a crossing is considered
    int lineCrossDistance = 20;
    // Whether to attempt to track people using a CSRT tracker between frames
    bool useTracking = false;
};

TrackerConfiguration *loadTrackerConfig();
void writeTrackerConfig(const TrackerConfiguration &conf);

template<typename T>
void log(const T &text) {
    std::cout << "BeeBit: " << text << std::endl;
}

}