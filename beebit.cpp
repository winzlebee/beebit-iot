#ifdef BEEBIT_PI
#include <raspicam/raspicam_cv.h>
#endif

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

PeopleCounter::PeopleCounter(int cameraId) : m_config(loadConfig()), m_imgSize(cv::Size(m_config->imageWidth, m_config->imageHeight)) {

    log("Loading Model");

    m_network = std::make_unique<BeeNet>(m_config);
    cv::ocl::setUseOpenCL(m_config->useOpenCL);

    log("Opening Camera");
    m_capture.open(cameraId);

    m_capture.set(cv::CAP_PROP_FRAME_WIDTH, m_imgSize.width);
    m_capture.set(cv::CAP_PROP_FRAME_HEIGHT, m_imgSize.height);

    // Initialize the BeeBit tracker
    log("Initializing tracker");
    m_tracker = std::make_unique<CentroidTracker>(m_config->maxDisappeared, m_config->searchDistance);

    totalUp = 0;
    totalDown = 0;
    m_totalFrames = 0;

}

void PeopleCounter::begin() {
    // Start the tracking process
    // TODO: Move tracking to a separate thread.
    cv::UMat frame;

    cv::namedWindow("BeeTrack");

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

        char key = cv::waitKey(5) & 0xFF;
        if (key == 'q') break;

        m_totalFrames += 1;
    }

    log("Main loop exited.");
}

void PeopleCounter::loop(cv::UMat &frame, double delta) {

    m_capture >> frame;

    std::vector<cv::Rect> trackedRects;

    if (m_totalFrames % m_config->skipFrames == 0) {
        m_trackers.clear();

        std::vector<cv::Rect> detections = m_network->getDetections(frame, m_imgSize);

        for (const auto &rect : detections) {
            if (m_showBoxes) cv::rectangle(frame, rect, netRectColor, 4);

            // Generate a tracker and add it to the list of trackers
            if (m_config->useCSRT) {
                m_trackers.push_back(cv::TrackerCSRT::create());
            } else {
                m_trackers.push_back(cv::TrackerKCF::create());
            }
            m_trackers.back()->init(frame, rect);
        }   
    } else {
        for (const auto &tracker : m_trackers) {

            cv::Rect2d trackerRect;
            tracker->update(frame, trackerRect);
            trackedRects.push_back(trackerRect);
        }
    }

    // Initialize a line for detection and determine if the tracked objects have passed the line
    cv::Vec2f lineVec;
    if (m_trackLine) {
        cv::line(frame, normalToScreen(m_lineStart, m_imgSize), normalToScreen(m_lineEnd, m_imgSize), cv::Scalar(255, 0, 255), 2);
        
        lineVec = m_lineEnd - m_lineStart;
        cv::normalize(lineVec);
        lineVec = perpendicular(lineVec);
    }

    // Update the tracker with the new information
    auto objects = m_tracker->update(trackedRects);

    for (const auto &trackedPerson : objects) {
        const auto personIndex = std::find_if(m_objects.begin(), m_objects.end(), [&](const TrackableObject &ob) {
            return ob.objectId == trackedPerson.first;
        });

        const bool personExists = personIndex != m_objects.end();
        
        if (m_trackLine) {
            cv::line(frame, getPointLineIntersect(m_lineStart, m_lineEnd, trackedPerson.second, lineVec), trackedPerson.second, cv::Scalar(255, 0, 0), 1);
        }

        if (personExists) {
            TrackableObject &ob = *personIndex;

            // Determine the direction the tracked object is travelling by comparing it to the running average
            cv::Point2i sum = std::accumulate(ob.centroids.begin(), ob.centroids.end(), cv::Point2i(0, 0), std::plus<cv::Point2i>());
            cv::Point2i mean(sum.x / ob.centroids.size(), sum.y / ob.centroids.size());
            cv::Point2i diff(mean - trackedPerson.second);
            cv::Vec2f dirVector = cv::normalize(cv::Vec2f(diff.x, diff.y));

            float direction = dirVector.dot(lineVec);

            ob.direction = direction;
            ob.centroids.push_back(trackedPerson.second);

            // Count the person
            if (!ob.counted) {
                float lineDistance = pointLineDist(m_lineStart, m_lineEnd, trackedPerson.second);
                ob.distance = lineDistance; 

                if (direction < 0 && lineDistance < 0) {
                    totalUp += 1;
                    ob.counted = true;
                } else if (direction > 0 && lineDistance > 0) {
                    totalDown += 1;
                    ob.counted = true;
                }
            }
        } else {
            TrackableObject personObject(trackedPerson.first, trackedPerson.second);
            m_objects.push_back(personObject);
        }

        std::string pointIdText("ID: ");
        pointIdText += std::to_string(trackedPerson.first);
        
        cv::circle(frame, trackedPerson.second, 4, (0, 0, 255), -1);
        cv::putText(frame, pointIdText, (trackedPerson.second - cv::Point2i(10, 10)), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0));

        // If we're showing rectangles, then show the tracked locations
        if (m_showBoxes) {
            for (const auto &rect : trackedRects) {
                cv::rectangle(frame, rect, rectColor, 2);
            }
        }

        if (m_trackLine) {
            std::string totalCount("Count: ");
            totalCount += std::to_string(totalDown + totalUp);
            cv::putText(frame, totalCount, cv::Point(10, m_imgSize.height - 20), cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 128, 128), 2);
        }

    }
    
    cv::imshow("BeeTrack", frame);

}

void PeopleCounter::setCountLine(const cv::Point2f &a, const cv::Point2f &b) {
    m_lineStart = a;
    m_lineEnd = b;
    enableCountLine();
}

void PeopleCounter::setCountLine(float startx, float starty, float endx, float endy) {
    setCountLine(cv::Point2f(startx, starty), cv::Point2f(endx, endy));
}

void PeopleCounter::setBoxes(bool boxes) {
    m_showBoxes = boxes;
}

bool PeopleCounter::lineInitialized() {
    return cv::norm(m_lineEnd - m_lineStart) > 1e-8;
}

void PeopleCounter::enableCountLine() {
    if (!lineInitialized()) return;
    m_trackLine = true;
}

void PeopleCounter::disableCountLine() {
    m_trackLine = false;
}

PeopleCounter::~PeopleCounter() {
    cv::destroyAllWindows();
}

int PeopleCounter::getCurrentCount() {
    return 0;
}

}

