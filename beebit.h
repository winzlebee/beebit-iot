#pragma once

#include "tracking/trackable_object.h"

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/tracking.hpp>

#include <memory>
#include <string>

namespace std {
    class thread;
}

namespace beebit {

struct TrackerConfiguration;
class CentroidTracker;
class BeeNet;

class PeopleCounter
{
public:
    PeopleCounter(int cameraIndex);
    ~PeopleCounter();

    // Begin the people counting operation
    std::thread *begin();

    
    // Set the line to count people walking past. Normalized in screen co-ordinates.
    /*void setCountLine(float startx, float starty, float endx, float endy);
    void setCountLine(const cv::Point2f &start, const cv::Point2f &end);

    void enableCountLine();
    void disableCountLine();*/

    void setDebugWindow(bool debug);

    // Retrieve the current count, as detected by the camera
    //int getCurrentCount();

private:
    void startThread(int camIndex, bool debugWindow);

    TrackerConfiguration *m_config;

    int m_cameraIndex;
    bool m_debug;
};

}