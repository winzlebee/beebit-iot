
#include "daemon.h"

extern "C" {
#include <curl/curl.h>
}

#include <thread>
#include <atomic>
#include <functional>

#include "../bee_util.h"
#include "../beebit.h"
#include "../util/types.h"

namespace beebit {

const std::string configLoc = "beemon.cfg";
std::atomic<bool> netThread = true;

Daemon::Daemon(const std::string &endpoint)
    : m_lifetime(0)
    , m_config(readConfiguration(configLoc))
{
}

Daemon::~Daemon() {
}

bool Daemon::onDetection(const DetectionResult result) {
    log(result.first);

    return netThread;
}

void Daemon::networkThread() {

    // Responsible for sending a count update at the specified interval
    std::chrono::seconds delayTime(20);
    std::string uuid;
    try {
        delayTime = std::chrono::seconds(std::get<int>(m_config.at("frequency")));
        uuid = std::get<std::string>(m_config.at("uuid"));
    } catch (const std::bad_variant_access &ex) {
        log("Invalid delay/uuid in config. Exiting network thread...");
        netThread = false;
    }

    while (netThread) {

        // Perform a CURL update against the network, using the current UUID
        {

        }

        log("Network Message sent.");
        std::this_thread::sleep_for(delayTime);
    }
    
}

void Daemon::start() {

    std::size_t cameraIndex;

    try {
        cameraIndex = std::get<int>(m_config.at("camera_index"));
    } catch (std::bad_variant_access &ex) {
        log("Invalid config camera index. Using default.");
        cameraIndex = 0;
    }

    auto detectFunc = std::bind(&Daemon::onDetection, this, std::placeholders::_1);

    beebit::PeopleCounter peopleCounter(cameraIndex, detectFunc);
	//peopleCounter.setDebugWindow(true);
	//peopleCounter.setCountLine(0, 0, 1.0f, 1.0f);
	
    m_networkThread = std::make_unique<std::thread>(&Daemon::networkThread, this);
	peopleCounter.begin();

    // We've exited the main thread, so terminate the network thread
    netThread = false;
    m_networkThread->join();

}

}