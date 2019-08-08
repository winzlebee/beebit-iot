#pragma once

#include <opencv2/opencv.hpp>

namespace beebit {

struct Configuration;

class PeopleCounter
{
public:
    PeopleCounter(int cameraId);
    ~PeopleCounter();
    void setCountLine(const cv::Point &start, const cv::Point &end);
    int getCurrentCount();
private:
    Configuration *m_config;
};

}