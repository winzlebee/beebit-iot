#pragma once

#include <opencv2/core/types.hpp>

namespace beebit {

// Class representing an object that can be tracked in the scene using the CentroidTracker
struct TrackableObject {

    TrackableObject(int id, const cv::Point2i &center) : objectId(id), centroid(center) {}

    int objectId;
    float direction;
    float distance;
    cv::Point2i centroid;
    bool counted = false;
};

}