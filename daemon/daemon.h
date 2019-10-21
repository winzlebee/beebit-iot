
#pragma once

#include <string>
#include <memory>
#include <chrono>
#include <exception>

#include "../util/types.h"

namespace std {
class thread;
}

namespace beebit {

class DaemonException : public std::exception {
public:
    explicit DaemonException(const std::string& message) :
      msg_(message)
      {}

    virtual ~DaemonException() throw (){}

    virtual const char* what() const throw (){
       return msg_.c_str();
    }
private:
    std::string msg_;
};

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
    Daemon();
    ~Daemon();

    /**
     * @brief Begin running the daemon
     * 
     */
    void start();

private:
    void networkThread();

    bool onDetection(const DetectionResult result);

    ConfigMap m_config;
    uint32_t m_lifetime;

    // The thread where we send data collected from the counter
    std::unique_ptr<std::thread> m_networkThread;

};

}