
#include "daemon.h"

#include <curl/curl.h>
#include <thread>
#include <atomic>

#include "../bee_util.h"
#include "../beebit.h"

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

void Daemon::networkThread() {

    // Responsible for sending a count update at the specified interval
    std::chrono::seconds delayTime(std::get<int>(m_config.at("frequency")));

    while (netThread) {
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


    beebit::PeopleCounter peopleCounter(cameraIndex);
	peopleCounter.setDebugWindow(true);
	//peopleCounter.setCountLine(0, 0, 1.0f, 1.0f);
	
    m_networkThread = std::make_unique<std::thread>(&Daemon::networkThread, this);
	peopleCounter.begin();

    // We've exited the main thread, so terminate the network thread
    netThread = false;
    m_networkThread->join();

}

}