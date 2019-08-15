
#include "centroid_tracker.h"

namespace beebit {

bool contains(const std::vector<int> &list, const int &num) {
    return std::find(std::begin(list), std::end(list), num) != std::end(list);
}

CentroidTracker::CentroidTracker(int maxDisappeared, int maxDistance)
 : m_maxDisappeared(maxDisappeared)
 , m_maxDistance(maxDistance)
{
}

void CentroidTracker::registerCentroid(const cv::Point2i &centroid) {
    m_objects.insert({m_nextObjectId, centroid});
    m_disappearedTime.insert({m_nextObjectId, 0});
    m_nextObjectId += 1;
}

void CentroidTracker::deregisterCentroid(const int centroidId) {
    // Erase the centroid with the specified ID
    m_objects.erase(centroidId);
    m_disappearedTime.erase(centroidId);
}

const std::map<int, cv::Point2i> &CentroidTracker::update(const std::vector<cv::Rect> &boxes) {
    if (boxes.empty()) {
        std::vector<int> markedForRemoval;
        for (auto &pair : m_disappearedTime) {
            // Add this frame where we haven't detected the object
            m_disappearedTime[pair.first] += 1;
            if (m_disappearedTime[pair.first] > m_maxDisappeared) {
                markedForRemoval.push_back(pair.first);
            }
        }

        for (const int &remove : markedForRemoval) {
            deregisterCentroid(remove);
        }

        return m_objects;
    }
    
    // Initialize an empty list of points representing the centroids
    std::vector<cv::Point2i> centroids;
    centroids.reserve(boxes.size());

    for (int i = 0; i < boxes.size(); i++) {
        int centerX = boxes[i].x + boxes[i].width/2;
        int centerY = boxes[i].y + boxes[i].height/2;
        centroids.push_back(cv::Point2i(centerX, centerY));
    }

    if (m_objects.empty()) {
        // Register all the input centroids we got from the rectangles
        std::for_each(centroids.begin(), centroids.end(), [&](const cv::Point2i &point) {
            registerCentroid(point);
        });

        return m_objects;
    }

    std::vector<int> usedExisting;
    std::vector<int> usedInput;

    // Gets the ID of the new centroid that was closest to the given centroid
    auto getMinDistInputId = [&, centroids](const cv::Point2i &point)->std::pair<int, double> {
        double closestDistance = INT_MAX;
        int id = -1;
        for (int i = 0; i < centroids.size(); i++) {
            double dist = cv::norm(point - centroids[i]);
            if (dist < closestDistance) {
                closestDistance = dist;
                id = i;
            }
        }
        return {id, closestDistance};
    };  

    for (const auto &existingObject : m_objects) {
        
        if (contains(usedExisting, existingObject.first)) {
            // If we've already got a marker for this centroid, then continue
            continue;
        }

        const auto &existingCentroid = existingObject.second;

        auto result = getMinDistInputId(existingCentroid);

        if (result.second > m_maxDistance) continue;
        if (contains(usedInput, result.first)) continue;

        // Reset the disappeared counter and set this objects new centroid
        m_disappearedTime[existingObject.first] = 0;
        m_objects[existingObject.first] = centroids[result.first];

        usedExisting.push_back(existingObject.first);
        usedInput.push_back(result.first);
    }

    // If there's more existing centroids than new ones
    // Then we need to check if any of these objects have disappeared.
    if (m_objects.size() > centroids.size()) {
        for (const auto &existingObject : m_objects) {
            if (!contains(usedExisting, existingObject.first)) {
                m_disappearedTime[existingObject.first] += 1;

                if (m_disappearedTime[existingObject.first] > m_maxDisappeared) {
                    deregisterCentroid(existingObject.first);
                }
            }
        }
    } else {
        // Every new input centroid is a new trackable object.
        for (int i = 0; i < centroids.size(); i++) {
            if (!contains(usedInput, i)) {
                registerCentroid(centroids[i]);
            }
        }
    }

    return m_objects;

}

}