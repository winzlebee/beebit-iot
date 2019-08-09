#ifdef BEEBIT_PI
#include <raspicam/raspicam_cv.h>
#endif

#include "beebit.h"
#include "bee_util.h"

namespace beebit {

PeopleCounter::PeopleCounter(int cameraId) {
    m_config = loadConfig();

    log("Loading Model");
    m_network = std::make_unique<cv::dnn::Net>(cv::dnn::readNetFromDarknet(m_config->configLocation, m_config->modelLocation));
    m_capture = std::make_unique<cv::VideoCapture>(cameraId);
    
}

void PeopleCounter::begin() {

}

PeopleCounter::~PeopleCounter() {

}

int PeopleCounter::getCurrentCount() {
    return 0;
}

}

