
#pragma once

#include <string>

namespace beebit {

/**
 * @brief The beebit daemon is responsible for handling the current state of the device and changing any settings as necessary.
 * 
 */
class Daemon {

public:
    /**
     * @brief Construct a new Daemon object
     * 
     * @param endpoint The URL that will be used for all queries.
     */
    Daemon(const std::string &endpoint);

};

}