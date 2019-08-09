#ifdef BEEBIT_PI
#include <raspicam/raspicam_cv.h>
#endif

#define PERSON_CLASS 1

#include "beebit.h"
#include "bee_util.h"
#include "tracking/centroid_tracker.h"

#include <chrono>

namespace beebit {

const float dnnScale = 0.00392f;
const cv::Scalar rectColor(255, 0, 0);

// Take a blob generated on an output for a Neural Network and generate Rectangles for the defined class
std::vector<cv::Rect> blobToRects(const cv::Mat &frame, const std::vector<cv::Mat> &blob, cv::dnn::Net *net, const Configuration *config) {
    static std::vector<int> outLayers = net->getUnconnectedOutLayers();
    static std::string outLayerType = net->getLayer(outLayers[0])->type;

    log(outLayerType);

    std::vector<cv::Rect> boxes;
    if (outLayerType == "DetectionOutput") {
        // Network produces an output blob, we can process for detections
        assert(blob.size() > 0);
        for (size_t k = 0; k < blob.size(); k++) {
            float *data = (float*) blob[k].data;
            for (size_t i = 0; i < blob[k].total(); i += 7) {
                float confidence = data[i + 2];
                if (confidence > config->confidence && ((int) (data[i+1])-1 == PERSON_CLASS)) {
                    int left   = (int)data[i + 3];
                    int top    = (int)data[i + 4];
                    int right  = (int)data[i + 5];
                    int bottom = (int)data[i + 6];
                    int width  = right - left + 1;
                    int height = bottom - top + 1;
                    if (width * height <= 1) {
                        // We're dealing with normalized detection locations
                        left   = (int)(data[i + 3] * frame.cols);
                        top    = (int)(data[i + 4] * frame.rows);
                        right  = (int)(data[i + 5] * frame.cols);
                        bottom = (int)(data[i + 6] * frame.rows);
                        width  = right - left + 1;
                        height = bottom - top + 1;
                    }
                    boxes.push_back(cv::Rect(left, top, width, height));
                }
            }
        }
    }
    return boxes;
}

PeopleCounter::PeopleCounter(int cameraId) {
    m_config = loadConfig();

    log("Loading Model");
    m_network = std::make_unique<cv::dnn::Net>(cv::dnn::readNetFromDarknet(m_config->configLocation, m_config->modelLocation));
    m_capture = std::make_unique<cv::VideoCapture>(cameraId);

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
        loop(frame, deltaTime.count());
        auto end = std::chrono::high_resolution_clock::now();
        deltaTime = end-start;

        char key = cv::waitKey(1) & 0xFF;
        if (key == 'q') break;

        m_totalFrames += 1;
    }
}

void PeopleCounter::loop(cv::Mat &frame, double delta) {

    *m_capture >> frame;
    cv::resize(frame, frame, cv::Size(m_camera.width, m_camera.height));
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

    m_statusText = "Waiting";

    //if (m_totalFrames % m_config->skipFrames == 0) {
    // We're in a detection frame
    m_statusText = "Detecting";

    cv::Mat blob = cv::dnn::blobFromImage(frame, dnnScale, cv::Size(416, 416), cv::Scalar(0, 0, 0));
    m_network->setInput(blob);
    cv::Mat forwardPass = m_network->forward();
    
    std::vector<cv::Rect> detections = blobToRects(frame, blob, m_network.get(), m_config);

    for (const auto &rect : detections) {
        cv::rectangle(frame, rect, rectColor, 4);
    }

    cv::imshow("BeeTrack", frame);
        
    //}

}

PeopleCounter::~PeopleCounter() {

}

int PeopleCounter::getCurrentCount() {
    return 0;
}

}

