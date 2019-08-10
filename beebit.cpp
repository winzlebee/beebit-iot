#ifdef BEEBIT_PI
#include <raspicam/raspicam_cv.h>
#endif

#define PERSON_CLASS 0

#include "beebit.h"
#include "bee_util.h"
#include "tracking/centroid_tracker.h"

#include <chrono>

namespace beebit {

const float dnnScale = 0.00392f;
const cv::Scalar rectColor(255, 0, 0);
const cv::Size darknetSize(416, 416);

// Take a blob generated on an output for a Neural Network and generate Rectangles for the defined class
std::vector<cv::Rect> blobToRects(const cv::Mat &frame, const std::vector<cv::Mat> &blob, cv::dnn::Net *net, const Configuration *config) {
    static std::vector<int> outLayers = net->getUnconnectedOutLayers();
    static std::string outLayerType = net->getLayer(outLayers[0])->type;

    std::vector<cv::Rect> boxes;
    if (outLayerType == "Region") {
        // Network produces an output blob, we can process for detections
        for (size_t i = 0; i < blob.size(); i++) {
            float *data = (float*) blob[i].data;
            for (size_t j = 0; j < blob[i].rows; ++j, data += blob[i].cols) {
                cv::Mat scores = blob[i].row(j).colRange(5, blob[i].cols);
                cv::Point classIdPoint;
                double confidence;
                minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
                if (confidence > config->confidence && classIdPoint.x == PERSON_CLASS) {
                    int centerX = (int)(data[0] * frame.cols);
                    int centerY = (int)(data[1] * frame.rows);
                    int width = (int)(data[2] * frame.cols);
                    int height = (int)(data[3] * frame.rows);
                    int left = centerX - width / 2;
                    int top = centerY - height / 2;

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

    m_network = cv::dnn::readNetFromDarknet(m_config->configLocation, m_config->modelLocation);
    m_capture.open(cameraId);

    m_outLayerNames = m_network.getUnconnectedOutLayersNames();

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

    if (frame.empty()) return;
 
    cv::resize(frame, frame, darknetSize);

    m_statusText = "Waiting";

    //if (m_totalFrames % m_config->skipFrames == 0) {

    // We're in a detection frame
    m_statusText = "Detecting";

    cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, darknetSize, cv::Scalar(), true, false, CV_8U);

    m_network.setInput(blob, "", dnnScale);

    std::vector<cv::Mat> forwardPass;
    m_network.forward(forwardPass, m_outLayerNames);
    
    std::vector<cv::Rect> detections = blobToRects(frame, forwardPass, &m_network, m_config);

    log(detections.size());
    for (const auto &rect : detections) {
        cv::rectangle(frame, rect, rectColor, 4);
    }
    

    cv::imshow("BeeTrack", frame);
        
    //}

}

PeopleCounter::~PeopleCounter() {
    cv::destroyAllWindows();
}

int PeopleCounter::getCurrentCount() {
    return 0;
}

}

