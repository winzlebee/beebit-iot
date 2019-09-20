
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
typedef std::map<std::string, std::variant<int, float, std::string> > ConfigMap;

typedef std::function<void(int, std::chrono::system_clock::time_point)> DetectionCallback;

}