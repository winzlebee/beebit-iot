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
    PeopleCounter(int cameraId);
    ~PeopleCounter();

    // Begin the people counting operation
    void begin();
    std::thread beginThreaded();

    // Set the line to count people walking past. Normalized in screen co-ordinates.
    void setCountLine(float startx, float starty, float endx, float endy);
    void setCountLine(const cv::Point2f &start, const cv::Point2f &end);

    void enableCountLine();
    void disableCountLine();

    void setDebugWindow(bool debug);

    // Retrieve the current count, as detected by the camera
    int getCurrentCount();
private:
    void loop(cv::UMat &frame, double delta);
    void getBoxes(const cv::UMat &frame, std::vector<cv::Rect> &detections);

    bool lineInitialized() const;

    /**
     * @brief Adds debug information represented by this people tracker to the given frame
     * 
     * @param frame The image to add debug info to
     */
    void showDebugInfo(cv::UMat &frame);

    TrackerConfiguration *m_config;
    const cv::Size m_imgSize;

    cv::VideoCapture m_capture;

    // Network and tracking
    std::unique_ptr<BeeNet> m_network;
    std::unique_ptr<CentroidTracker> m_tracker;

    // The trackers that track individual people in the frame
    std::vector<cv::Ptr<cv::Tracker> > m_trackers;

    // The objects that are currently being tracked
    std::vector<TrackableObject> m_objects;

    // Current position of the count line
    cv::Point2f m_lineStart;
    cv::Point2f m_lineEnd;
    bool m_trackLine = false;

    bool m_debug = false;

    // Counters
    uint32_t totalUp;
    uint32_t totalDown;

    uint64_t m_totalFrames;
};

}