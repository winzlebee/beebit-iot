#ifdef BEEBIT_PI
#include <raspicam/raspicam_cv.h>
#endif

#include "beebit.h"
#include "bee_util.h"

#include "net/beenet.h"
#include "tracking/centroid_tracker.h"

#include <chrono>

namespace beebit {

const cv::Scalar rectColor(255, 0, 0);

PeopleCounter::PeopleCounter(int cameraId) {
    m_config = loadConfig();

    log("Loading Model");

    m_network = std::make_unique<BeeNet>(m_config);
    m_capture.open(cameraId);

    // Initialize the BeeBit tracker
    m_tracker = std::make_unique<CentroidTracker>(40, 50);

    totalUp = 0;
    totalDown = 0;
    m_totalFrames = 0;

}

void PeopleCounter::begin() {
    // Start the tracking process
    // TODO: Move tracking to a separate thread.
    cv::Mat frame;

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
}

void PeopleCounter::loop(cv::Mat &frame, double delta) {

    m_capture >> frame;

    if (m_config->skipFrames % m_totalFrames == 0) {
        std::vector<cv::Rect> detections = m_network->getDetections(frame);

        for (const auto &rect : detections) {
            cv::rectangle(frame, rect, rectColor, 4);
        }
    }
    
    cv::imshow("BeeTrack", frame);

}

PeopleCounter::~PeopleCounter() {
    cv::destroyAllWindows();
}

int PeopleCounter::getCurrentCount() {
    return 0;
}

}

