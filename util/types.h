
#include <map>
#include <string>
#include <variant>

namespace beebit {

/**
 * @brief A set of name/value pairs representing config values
 * 
 */
typedef std::map<std::string, std::variant<int, float, std::string> > ConfigMap;

}