
#include "beenet.h"
#include "../bee_util.h"

#include <iostream>

namespace beebit {

const float dnnScale = 0.00392f;

// Take a blob generated on an output for a Neural Network and generate Rectangles for the defined class
std::vector<cv::Rect> BeeNet::blobToRects(const cv::Mat &frame, const std::vector<cv::Mat> &blob, const cv::Size &finalSize) {
    static std::vector<int> outLayers = m_network->getUnconnectedOutLayers();
    static std::string outLayerType = m_network->getLayer(outLayers[0])->type;

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
                if (confidence > m_config->confidence && classIdPoint.x == PERSON_CLASS) {
                    int centerX = (int)(data[0] * frame.cols);
                    int centerY = (int)(data[1] * frame.rows);
                    int width = (int)(data[2] * frame.cols);
                    int height = (int)(data[3] * frame.rows);
                    int left = centerX - width / 2;
                    int top = centerY - height / 2;
                    
                    boxes.push_back(cv::Rect( float(left)/m_netSize.width * finalSize.width,
                                              float(top)/m_netSize.height * finalSize.height,
                                              float(width)/m_netSize.width * finalSize.width,
                                              float(height)/m_netSize.height * finalSize.height ));
                }
            }
        }
    }
    return boxes;
}

BeeNet::BeeNet(const TrackerConfiguration *config) : m_config(config), m_netSize(cv::Size(config->neuralNetQuality, config->neuralNetQuality)) {
    m_network = std::make_unique<cv::dnn::Net>(cv::dnn::readNetFromDarknet(m_config->configLocation, m_config->modelLocation));

    m_outputLayerNames = m_network->getUnconnectedOutLayersNames();

}

BeeNet::~BeeNet() {
    
}

std::vector<cv::Rect> BeeNet::getDetections(const cv::Mat &frame, const cv::Size &screenSize) {
    cv::Mat detectFrame;
    cv::resize(frame, detectFrame, m_netSize);

    if (frame.empty()) return {};

    cv::Mat blob = cv::dnn::blobFromImage(frame, 1.0, m_netSize, cv::Scalar(), true, false, CV_8U);
    m_network->setInput(blob, "", dnnScale);

    std::vector<cv::Mat> forwardPass;
    m_network->forward(forwardPass, m_outputLayerNames);

    std::vector<cv::Rect> rects = blobToRects(detectFrame, forwardPass, screenSize);

    return rects;
}

cv::dnn::Net *BeeNet::getNetwork() {
    return m_network.get();
}

}