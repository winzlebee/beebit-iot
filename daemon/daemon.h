
#pragma once

#include <string>

#include "../util/types.h"

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
    ~Daemon();

    /**
     * @brief Begin running the daemon
     * 
     */
    void start();

private:
    ConfigMap m_config;
    uint32_t m_lifetime;

};

}