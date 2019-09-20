#pragma once

#include "tracking/trackable_object.h"
#include "util/types.h"

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/tracking.hpp>

#include <memory>
#include <string>

namespace beebit {

struct TrackerConfiguration;
class CentroidTracker;
class BeeNet;
class PeopleCounterImpl;

class PeopleCounter
{
public:
    PeopleCounter(int cameraIndex, DetectionCallback callback);
    ~PeopleCounter();

    // Begin the people counting operation
    void begin();
    
    // Set the line to count people walking past. Normalized in screen co-ordinates.
    void setCountLine(float startx, float starty, float endx, float endy);
    void setCountLine(const cv::Point2f &start, const cv::Point2f &end);

    void enableCountLine();
    void disableCountLine();

    void setDebugWindow(bool debug);

private:

    TrackerConfiguration *m_config;
    std::unique_ptr<PeopleCounterImpl> m_impl;

    int m_cameraIndex;
    bool m_debug;
};

}