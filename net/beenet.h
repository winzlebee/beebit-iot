#pragma once

#define PERSON_CLASS 0

#include <memory>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

namespace beebit {

class Configuration;

// The BeeBit neural network module. Currently powered by Darknet
class BeeNet {
public:
    
    // Initialize the BeeBit neural network with a config file
    BeeNet(const Configuration *config);
    ~BeeNet();

    // Makes detections given an image frame. Outputs normalized rect coordinates.
    std::vector<cv::Rect> getDetections(cv::Mat &frame);

    cv::dnn::Net *getNetwork();
private:
    std::unique_ptr<cv::dnn::Net> m_network;
    std::vector<std::string> m_outputLayerNames;

    const Configuration *m_config;
};

}