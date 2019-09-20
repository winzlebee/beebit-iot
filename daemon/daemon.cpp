
#include "daemon.h"

#include <curl/curl.h>
#include <thread>

#include "../bee_util.h"
#include "../beebit.h"

namespace beebit {

const std::string configLoc = "beemon.cfg";

Daemon::Daemon(const std::string &endpoint)
    : m_lifetime(0)
    , m_config(readConfiguration(configLoc))
{
}

Daemon::~Daemon() {
    m_countThread->join();
}

void Daemon::networkThread() {
    
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
	
	peopleCounter.begin();

    log("Count thread started.");
}

}