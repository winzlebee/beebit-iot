#pragma once

#include <opencv2/core/types.hpp>

namespace beebit {

// Class representing an object that can be tracked in the scene using the CentroidTracker
struct TrackableObject {

    TrackableObject(int id, const cv::Point2i &center) : objectId(id) {
        centroids.push_back(center);
    }

    int objectId;
    float direction;
    float distance;
    std::vector<cv::Point2i> centroids;
    bool counted = false;
};

}