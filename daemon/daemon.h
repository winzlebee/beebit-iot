
#pragma once

#include <string>
#include <memory>
#include <chrono>

#include "../util/types.h"

namespace std {
class thread;
}

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
    void networkThread();
    void onDetection(int count, std::chrono::system_clock::time_point timePoint);

    ConfigMap m_config;
    uint32_t m_lifetime;

    // The thread where we send data collected from the counter
    std::unique_ptr<std::thread> m_networkThread;

};

}