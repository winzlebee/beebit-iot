#pragma once

#include <map>
#include <vector>

#include <opencv2/opencv.hpp>

namespace beebit {

// Tracks items using OpenCV
class CentroidTracker {

public:
    
    // Tracker attributes can be changed to improve performance.
    // maxDisappeared: The number of frames before a tracker is considered 'disappeared'
    // maxDistance: The maximum search distance for the centroid tracker
    CentroidTracker(int maxDisappeared = 50, int maxDistance = 50);

    // Register a centroid to be tracked
    void registerCentroid(const cv::Point2i &centroid);

    // Deregister a centroid and stop tracking it
    void deregisterCentroid(const int centroidId);

    // Update the tracker with the new bounding boxes represented as rects
    const std::map<int, cv::Point2i> &update(const std::vector<cv::Rect> &boxes);

private:
    int m_maxDisappeared;
    int m_maxDistance;

    int m_nextObjectId = 0;

    // Map of centroids that are being tracked
    std::map<int, cv::Point2i> m_objects;
    std::map<int, int> m_disappearedTime;
    
};

}