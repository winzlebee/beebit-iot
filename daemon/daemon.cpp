
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
#include "../util/base64.h"

namespace beebit {

const std::string configLoc = "beemon.cfg";

std::atomic<bool> netThread(true);
std::atomic<bool> sendFrame(false);

std::mutex mut;

DetectionResult latestResult;

// Callback for results from CURL requests
size_t write_data(void *buffer, size_t size, size_t nmemb, void *thisDaemon)
{
    Daemon *daemon = static_cast<Daemon*>(thisDaemon);

    // Interpret the response as a state of whether to take a capture the next time we send data
    char *charBuff = static_cast<char*>(buffer);
    std::string recievedString(charBuff, nmemb);
    if (recievedString.empty()) return size * nmemb;

    std::istringstream configStream(recievedString);

    // Recieve the new configuration values
    ConfigMap returnedConfig = readConfiguration(configStream, '|');

    std::lock_guard<std::mutex> lk(mut);
    try {
        if (static_cast<bool>(stoi(returnedConfig.at("updateConfig")))) {
            loadTrackerConfigMap(returnedConfig);
            daemon->setConfig(returnedConfig);
            log("Config Updated");
        }

        sendFrame = static_cast<bool>(stoi(returnedConfig.at("sendFrame")));
    } catch (std::out_of_range &e) {
        log("Returned config didn't contain expected value.");
    }

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

void Daemon::setConfig(const ConfigMap &map) {
    m_config = map;
    m_configChanged = true;
}

Daemon::~Daemon() {
}

bool Daemon::onDetection(const DetectionResult result) {
    std::lock_guard<std::mutex> lk(mut);

    // TODO: Log results to a local database and cache them for when the network fails?
    latestResult = result;
    
    return netThread && !m_configChanged;
}


void Daemon::networkThread() {

    // Responsible for sending a count update at the specified interval
    std::chrono::seconds delayTime;
    std::string uuid;
    std::string endpoint;

    try {
        uuid = (m_config.at("uuid"));
        endpoint = (m_config.at("endpoint"));
    } catch (std::out_of_range &ex) {
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
            "\"people\":" << std::get<0>(latestResult).size() << ", " << 
            "\"timestamp\":" << std::chrono::duration_cast<std::chrono::seconds>(std::get<1>(latestResult).time_since_epoch()).count() << ", ";

        // Send all the IDs and their positions on the screen, normalized
        stream << "\"trackers\": [";
        std::vector<TrackableObject> &people = std::get<0>(latestResult);
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

        // If we have scheduled an image send, then encode the latest image capture and send it to the server
        if (sendFrame && !std::get<2>(latestResult).empty()) {
            cv::Mat resultCopy;
            std::get<2>(latestResult).copyTo(resultCopy);

            std::vector<int> compressionParams;
            compressionParams.push_back(cv::IMWRITE_JPEG_QUALITY);
            compressionParams.push_back(std::stoi(m_config.at("image_quality")));

            std::vector<uchar> frameBuffer;
            if (cv::imencode(".jpg", resultCopy, frameBuffer, compressionParams)) {
                log("Sent encoded image to server.");
                stream << "\"frame\": \"" << base64_encode(&frameBuffer[0], frameBuffer.size()) << "\", ";
            }
        }

        stream << "\"status\":\"Good\" }";
        std::string sendJson = stream.str();

        // Perform a CURL update against the network, using the current UUID
        CURL *curl = curl_easy_init();
        if (curl) {
            CURLcode response;
            curl_easy_setopt(curl, CURLOPT_URL, (endpoint + "/bee/update").c_str());
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sendJson.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
            response = curl_easy_perform(curl);

            if (response != CURLE_OK) {
                log("Failed to send data to server.");
            }

            curl_easy_cleanup(curl);
        }

        log("Network Message sent.");
        std::this_thread::sleep_for(std::chrono::seconds(std::stoi(m_config.at("frequency"))));
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
    m_peopleCounter = std::make_unique<beebit::PeopleCounter>(cameraIndex, detectFunc);
    m_peopleCounter->setDebugWindow(true);
    //peopleCounter.setCountLine(0, 0, 1.0f, 1.0f);
	
    m_networkThread = std::make_unique<std::thread>(&Daemon::networkThread, this);

    while (netThread) {
        // Keep re-starting detection while the network thread is active.
        // The only other reason the detection process would re-start is due to a config change.
	    m_peopleCounter->begin();
        
        // If something else caused the termination, then break.
        if (!m_configChanged && netThread) {
            break;
        }

        m_configChanged = false;
    }

    writeTrackerConfigFile();

    // We've exited the main thread, so terminate the network thread
    netThread = false;
    m_networkThread->join();

}

}
