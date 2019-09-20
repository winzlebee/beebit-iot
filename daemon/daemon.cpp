
#include "daemon.h"

#include <curl/curl.h>

#include <thread>
#include <atomic>
#include <functional>
#include <mutex>

#include "../bee_util.h"
#include "../beebit.h"
#include "../util/types.h"

namespace beebit {

const std::string configLoc = "beemon.cfg";

std::atomic<bool> netThread = true;

std::mutex mut;

DetectionResult latestResult;

Daemon::Daemon()
    : m_lifetime(0)
    , m_config(readConfiguration(configLoc))
{
}

Daemon::~Daemon() {
}

bool Daemon::onDetection(const DetectionResult result) {
    std::lock_guard<std::mutex> lk(mut);

    // TODO: Log results to a local database and cache them for when the network fails?
    latestResult = result;

    return netThread;
}

void Daemon::networkThread() {

    // Responsible for sending a count update at the specified interval
    std::chrono::seconds delayTime;
    std::string uuid;
    std::string endpoint;
    try {
        delayTime = std::chrono::seconds(std::get<int>(m_config.at("frequency")));
        uuid = std::get<std::string>(m_config.at("uuid"));
        endpoint = std::get<std::string>(m_config.at("endpoint"));
    } catch (const std::bad_variant_access &ex) {
        log("Invalid value in config. Exiting network thread...");
        netThread = false;
    }

    while (netThread) {

        // Perform a CURL update against the network, using the current UUID
        CURL *curl = curl_easy_init();
        if (curl) {
            CURLcode response;
            curl_easy_setopt(curl, CURLOPT_URL, (endpoint + "/bee/update").c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");
            response = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
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
	peopleCounter.setDebugWindow(true);
	//peopleCounter.setCountLine(0, 0, 1.0f, 1.0f);
	
    m_networkThread = std::make_unique<std::thread>(&Daemon::networkThread, this);
	peopleCounter.begin();

    // We've exited the main thread, so terminate the network thread
    netThread = false;
    m_networkThread->join();

}

}