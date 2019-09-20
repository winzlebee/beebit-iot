
#include <map>
#include <string>
#include <variant>
#include <chrono>
#include <functional>

namespace beebit {

/**
 * @brief A set of name/value pairs representing config values
 * 
 */
typedef std::map<std::string, std::variant<int, std::string> > ConfigMap;

/**
 * @brief Represents a single result of the detection process
 * 
 */
typedef std::pair<int, std::chrono::system_clock::time_point> DetectionResult;

/**
 * @brief Callback function to deliver detection results
 * 
 */
typedef std::function<bool(const DetectionResult)> DetectionCallback;

}