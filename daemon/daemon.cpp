
#include "daemon.h"

#include <curl/curl.h>

#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <cstring>
#include <algorithm>
#include <fstream>

#include "../bee_util.h"
#include "../beebit.h"
#include "../util/types.h"

namespace beebit {

const std::string configLoc = "beemon.cfg";

std::atomic<bool> netThread(true);

std::mutex mut;

DetectionResult latestResult;

// Callback for results from CURL requests
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    // Interpret the response as a state of whether to take a capture the next time we send data
    char *charBuff = static_cast<char*>(buffer);
    std::string recievedString(charBuff, nmemb);

    std::cout << "Response: " << recievedString << std::endl;

    // The number of bytes recieved by this transfer
    return size * nmemb;
}

Daemon::Daemon()
    : m_lifetime(0)
{
    std::ifstream configFile(configLoc);

    if (!configFile) {
        throw DaemonException("Config file invalid!");
        return;
    }

    m_config = readConfiguration(configFile, '\n');

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
        delayTime = std::chrono::seconds(std::stoi(m_config.at("frequency")));
        uuid = (m_config.at("uuid"));
        endpoint = (m_config.at("endpoint"));
    } catch (std::invalid_argument &ex) {
        log("Invalid value in config! Stopping network...");
        netThread = false;
    }

    curl_slist *header = nullptr;
    header = curl_slist_append(header, "Content-Type: application/json");
    //header = curl_slist_append(header, "charsets: utf-8");

    while (netThread) {

        std::stringstream stream;
        stream << "{ " << 
            "\"uuid\": \"" << uuid << "\", " << 
            "\"people\":" << latestResult.first.size() << ", " << 
            "\"timestamp\":" << std::chrono::duration_cast<std::chrono::seconds>(latestResult.second.time_since_epoch()).count() << ", ";

        // Send all the IDs and their positions on the screen, normalized
        stream << "\"trackers\": [";
        std::vector<TrackableObject> &people = latestResult.first;
        for (int i = 0; i < people.size(); i++) {
            stream << "{";
            stream << "\"id\":" << people[i].objectId << ", ";
            stream << "\"x\":" << people[i].centroids.back().x/((float) (TrackerConfiguration::instance()->imageWidth)) << ",";
            stream << "\"y\":" << people[i].centroids.back().y/((float) (TrackerConfiguration::instance()->imageHeight));
            stream << "}";
            
            if (i < (people.size() - 1)) {
                stream << ",";
            }
        }
        stream << "], ";

        stream << "\"status\":\"Good\" }";
        std::string sendJson = stream.str();

        // Perform a CURL update against the network, using the current UUID
        CURL *curl = curl_easy_init();
        if (curl) {
            CURLcode response;
            curl_easy_setopt(curl, CURLOPT_URL, (endpoint + "/bee/update").c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sendJson.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            response = curl_easy_perform(curl);

            if (response != CURLE_OK) {
                log("Failed to send data to server.");
            }

            curl_easy_cleanup(curl);
        }

        log("Network Message sent.");
        std::this_thread::sleep_for(delayTime);
    }
    
}

void Daemon::start() {

    std::size_t cameraIndex;

    try {
        cameraIndex = stoi(m_config.at("camera_index"));
    } catch (std::invalid_argument &ex) {
        log("Invalid config camera index. Using default.");
        cameraIndex = 0;
    }

    auto detectFunc = std::bind(&Daemon::onDetection, this, std::placeholders::_1);

    loadTrackerConfigFile();
    beebit::PeopleCounter peopleCounter(cameraIndex, detectFunc);
    peopleCounter.setDebugWindow(true);
    //peopleCounter.setCountLine(0, 0, 1.0f, 1.0f);
	
    m_networkThread = std::make_unique<std::thread>(&Daemon::networkThread, this);
	peopleCounter.begin();
    writeTrackerConfigFile();

    // We've exited the main thread, so terminate the network thread
    netThread = false;
    m_networkThread->join();

}

}
