#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

#ifdef BEEBIT_PI
#include <raspicam/raspicam_cv.h>
#endif

#include "beebit.h"
#include "bee_util.h"

namespace beebit {

PeopleCounter::PeopleCounter(int cameraId) {
    m_config = loadConfig();
    cv::VideoCapture capture(cameraId);

    log("Loading Model");
    cv::dnn::readNetFromDarknet(m_config->configLocation, m_config->modelLocation);

}

PeopleCounter::~PeopleCounter() {

}

int PeopleCounter::getCurrentCount() {
    return 0;
}

}

