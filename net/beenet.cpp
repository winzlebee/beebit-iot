
#include "beenet.h"
#include "../bee_util.h"

namespace beebit {

const cv::Size darknetSize(416, 416);
const float dnnScale = 0.00392f;

namespace {

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

}

BeeNet::BeeNet(const Configuration *config) : m_config(config) {
    m_network = std::make_unique<cv::dnn::Net>(cv::dnn::readNetFromDarknet(m_config->configLocation, m_config->modelLocation));

    m_outputLayerNames = m_network->getUnconnectedOutLayersNames();

}

BeeNet::~BeeNet() {
    
}

std::vector<cv::Rect> BeeNet::getDetections(cv::Mat &frame) {
    cv::resize(frame, frame, darknetSize);

    if (frame.empty()) return {};

    cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, darknetSize, cv::Scalar(), true, false, CV_8U);
    m_network->setInput(blob, "", dnnScale);

    std::vector<cv::Mat> forwardPass;
    m_network->forward(forwardPass, m_outputLayerNames);

    return blobToRects(frame, forwardPass, m_network.get(), m_config);
}

cv::dnn::Net *BeeNet::getNetwork() {
    return m_network.get();
}

}