#include <raspicam/raspicam_cv.h>

#include <opencv2/core/ocl.hpp>

#include "beebit.h"
#include "bee_util.h"

#include "net/beenet.h"
#include "tracking/centroid_tracker.h"
#include "tracking/trackable_object.h"

#include <chrono>
#include <numeric>

namespace beebit {

const cv::Scalar rectColor(255, 0, 0);
const cv::Scalar netRectColor(0, 255, 0);

cv::Point2i normalToScreen(const cv::Point2f &point, const cv::Size &img) {
    return cv::Point2i(point.x*img.width, point.y*img.height);
}

cv::Vec2f perpendicular(const cv::Vec2f &vector) {
    return cv::Vec2f(vector[1], -vector[0]);
}

// Get the distance between a point and a line
float pointLineDist(const cv::Point2i &l1, const cv::Point2i &l2, const cv::Point2i &point) {
    int x_diff = l2.x - l1.x;
    int y_diff = l2.y - l1.y;
    float num = -(y_diff*point.x - x_diff*point.y + l2.x*l1.y - l2.y*l1.x);
    return num / cv::sqrt(x_diff*x_diff + y_diff*y_diff);
}

// Returns the point of intersection between a line and a point
cv::Point2i getPointLineIntersect(const cv::Point2i &start, const cv::Point2i &end, const cv::Point2i &point, const cv::Vec2f &lineVec) {
    cv::Point2i intersect = point + cv::Point2i(lineVec * pointLineDist(start, end, point));
    return intersect;
}

/**
 * @brief Implementation of the people tracker, called by the exposed people tracker.
 * 
 */
class PeopleCounterImpl {
public:
    PeopleCounterImpl(int cameraIndex, const TrackerConfiguration *config, DetectionCallback cb)
        : m_config(config)
        , m_imgSize(cv::Size(m_config->imageWidth, m_config->imageHeight))
        , m_callback(cb) {

        log("Loading Model");

        m_network = std::make_unique<BeeNet>(m_config);
        cv::ocl::setUseOpenCL(m_config->useOpenCL);

        log("Opening Camera");
        m_capture.open();

        m_capture.set(cv::CAP_PROP_FRAME_WIDTH, m_imgSize.width);
        m_capture.set(cv::CAP_PROP_FRAME_HEIGHT, m_imgSize.height);

        // Initialize the BeeBit tracker
        log("Initializing tracker");
        m_tracker = std::make_unique<CentroidTracker>(m_config->maxDisappeared, m_config->searchDistance);

        totalUp = 0;
        totalDown = 0;
        m_totalFrames = 0;
    }

    void getBoxes(const cv::Mat &frame, std::vector<cv::Rect> &detections) {
        detections = m_network->getDetections(frame, m_imgSize);
    }

    void loop(cv::Mat &frame, double delta) {

        m_capture.grab();
        m_capture.retrieve(frame);

        std::vector<cv::Rect> trackedRects;

        getBoxes(frame, trackedRects);

        // Initialize a line for detection and determine if the tracked objects have passed the line
        cv::Vec2f lineVec;
        if (m_trackLine) {
            lineVec = m_lineEnd - m_lineStart;
            cv::normalize(lineVec);
            lineVec = perpendicular(lineVec);
        }

        // Update the tracker with the new information
        auto objects = m_tracker->update(trackedRects);

        std::size_t iter = 0;
        for (const auto &trackedPerson : objects) {
            const auto personIndex = std::find_if(m_objects.begin(), m_objects.end(), [&](const TrackableObject &ob) {
                return ob.objectId == trackedPerson.first;
            });

            const bool personExists = personIndex != m_objects.end();

            if (personExists) {
                TrackableObject &ob = *personIndex;

                if (m_trackLine) {

                    // Determine the direction the tracked object is travelling by comparing it to the running average
                    cv::Point2i sum = std::accumulate(ob.centroids.begin(), ob.centroids.end(), cv::Point2i(0, 0), std::plus<cv::Point2i>());
                    cv::Point2i mean(sum.x / ob.centroids.size(), sum.y / ob.centroids.size());
                    cv::Point2i diff(mean - trackedPerson.second);
                    cv::Vec2f dirVector = cv::normalize(cv::Vec2f(diff.x, diff.y));

                    float direction = dirVector.dot(lineVec);

                    ob.direction = direction;

                    // Count the person
                    if (!ob.counted) {
                        float lineDistance = std::abs(pointLineDist(m_lineStart, m_lineEnd, trackedPerson.second));
                        ob.distance = lineDistance; 

                        if (lineDistance < m_config->lineCrossDistance) {
                            if (direction < 0) {
                                totalUp += 1;
                            } else if (direction > 0) {
                                totalDown += 1;
                            }
                            ob.counted = true;
                        }
                    }
                }

                ob.centroids.push_back(trackedPerson.second);
            } else {
                TrackableObject personObject(trackedPerson.first, trackedPerson.second);
                m_objects.push_back(personObject);
            }
            iter++;
        }

        if (iter < m_objects.size()) {
            // Remove the people that are in m_objects but not in our new detection
            const auto range = std::remove_if(m_objects.begin(), m_objects.end(), [&](const TrackableObject &ob) {
                return objects.count(ob.objectId) == 0;
            });

            m_objects.erase(range);
        }

        if (m_debug) showDebugInfo(frame);
    }


    void begin() {
        // Start the tracking process
        cv::Mat frame;

        if (m_debug) cv::namedWindow("BeeTrack");

        std::chrono::duration<double> deltaTime(0);
        while (true) {
            auto start = std::chrono::high_resolution_clock::now();

            try {
                loop(frame, deltaTime.count());
            } catch (cv::Exception &ex) {
                std::cerr << ex.what() << std::endl;
                break;
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            deltaTime = end-start;

            if (!m_callback({m_objects.size(), end})) break;

            char key = cv::waitKey(5) & 0xFF;
            if (key == 'q') break;

            m_totalFrames += 1;
        }

        log("Main loop exited.");
    }

    void showDebugInfo(cv::Mat &frame) {

        // Show debug info for all our people
        for (const auto &object : m_objects) {
            if (object.centroids.empty()) continue;

            std::string pointIdText("ID: ");
            pointIdText += std::to_string(object.objectId);
            
            cv::circle(frame, object.centroids.back(), 4, (0, 0, 255), -1);
            cv::putText(frame, pointIdText, (object.centroids.back() - cv::Point2i(10, 25)), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0));

            if (m_trackLine) {
                std::string distText("Distance: ");
                distText += std::to_string(object.distance);
                cv::putText(frame, distText, (object.centroids.back() - cv::Point2i(10, 10)), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0));
            }
        }

        // Print our tracking line
        if (m_trackLine) {
            std::string totalCount("Count: ");
            totalCount += std::to_string(totalDown + totalUp);
            cv::putText(frame, totalCount, cv::Point(10, m_imgSize.height - 20), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 128, 128), 2);
            cv::line(frame, m_lineStart, m_lineEnd, cv::Scalar(255, 0, 255), 1);
        }

        cv::imshow("BeeTrack", frame);
    }

    void setCountLine(const cv::Point2f &a, const cv::Point2f &b) {
        m_lineStart = normalToScreen(a, m_imgSize);
        m_lineEnd = normalToScreen(b, m_imgSize);
        enableCountLine();
    }

    void setCountLine(float startx, float starty, float endx, float endy) {
        setCountLine(cv::Point2f(startx, starty), cv::Point2f(endx, endy));
    }

    void setDebugWindow(bool debug) {
        m_debug = debug;
    }

    bool lineInitialized() const {
        return cv::norm(m_lineEnd - m_lineStart) > 1e-8;
    }

    void enableCountLine() {
        if (!lineInitialized()) return;
        m_trackLine = true;
    }

    void disableCountLine() {
        m_trackLine = false;
    }

private:
    const TrackerConfiguration *m_config;
    const cv::Size m_imgSize;

    raspicam::RaspiCam_Cv m_capture;

    // Network and tracking
    std::unique_ptr<BeeNet> m_network;
    std::unique_ptr<CentroidTracker> m_tracker;

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

    // Callback when the count changes
    DetectionCallback m_callback;

};

PeopleCounter::PeopleCounter(int cameraIndex, DetectionCallback cb) : m_config(loadTrackerConfig()), m_cameraIndex(cameraIndex) {
    m_impl = std::make_unique<PeopleCounterImpl>(m_cameraIndex, m_config, cb);
}

PeopleCounter::~PeopleCounter() {
}

void PeopleCounter::begin() {
    m_impl->begin();
}

void PeopleCounter::setCountLine(float startx, float starty, float endx, float endy) {
    m_impl->setCountLine(startx, starty, endx, endy);
}

void PeopleCounter::setCountLine(const cv::Point2f &start, const cv::Point2f &end) {
    m_impl->setCountLine(start, end);
}

void PeopleCounter::enableCountLine() {
    m_impl->enableCountLine();
}

void PeopleCounter::disableCountLine() {
    m_impl->disableCountLine();
}

void PeopleCounter::setDebugWindow(bool debug) {
    m_impl->setDebugWindow(debug);
}

}

