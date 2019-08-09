#pragma once

#include "bee_camera.h"

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

#include <memory>

namespace beebit {

struct Configuration;

class PeopleCounter
{
public:
    PeopleCounter(int cameraId);
    ~PeopleCounter();

    // Begin the people counting operation
    void begin();

    // Set the line to count people walking past
    void setCountLine(const cv::Point &start, const cv::Point &end);

    // Retrieve the current count, as detected by the camera
    int getCurrentCount();
private:
    Configuration *m_config;

    std::unique_ptr<cv::VideoCapture> m_capture;
    std::unique_ptr<cv::dnn::Net> m_network;

    BeeCamera m_camera;
};

}