#pragma once


#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

#include <memory>
#include <string>

namespace beebit {

struct Configuration;
class CentroidTracker;
class BeeNet;

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

    std::unique_ptr<BeeNet> m_network;
    std::unique_ptr<CentroidTracker> m_tracker;

    // Counters
    uint32_t totalUp;
    uint32_t totalDown;

    uint32_t m_totalFrames;

    std::string m_statusText;
};

}