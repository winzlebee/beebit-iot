
#include "../tracking/trackable_object.h"

#include <map>
#include <string>
#include <chrono>
#include <functional>
#include <vector>

namespace beebit {

/**
 * @brief A set of name/value pairs representing config values
 * 
 */
typedef std::map<std::string, std::string > ConfigMap;

/**
 * @brief Represents a single result of the detection process
 * 
 */
typedef std::tuple<std::vector<TrackableObject>, std::chrono::system_clock::time_point, cv::Mat> DetectionResult;

/**
 * @brief Callback function to deliver detection results
 * 
 */
typedef std::function<bool(const DetectionResult)> DetectionCallback;

}