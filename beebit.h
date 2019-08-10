#pragma once

#include "bee_camera.h"

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

#include <memory>
#include <string>

namespace beebit {

struct Configuration;
class CentroidTracker;

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
    void loop(cv::Mat &frame, double delta);

    Configuration *m_config;

    cv::VideoCapture m_capture;
    cv::dnn::Net m_network;
    std::unique_ptr<CentroidTracker> m_tracker;

    std::vector<cv::String> m_outLayerNames;

    // Counters
    uint32_t totalUp;
    uint32_t totalDown;

    uint32_t m_totalFrames;

    std::vector<cv::Rect> m_detections;
    std::string m_statusText;

    BeeCamera m_camera;
};

}