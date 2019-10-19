#include "bee_util.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

namespace beebit {

static const std::string CONFIG_FILE_NAME = "beebit.cfg";

ConfigMap readConfiguration(const std::string &location) {

    std::ifstream inFile(location.c_str());
    ConfigMap readResult;

    if (!inFile) {
        return readResult;
    }

    // Read the lines of the config into the configuration map
    std::string currentLine;
    while (std::getline(inFile, currentLine)) {
        if (currentLine.at(0) == '#') continue;

        // For the current line, separate the configuration into a set of keys and values
        std::stringstream lineStream(currentLine);
        std::string segments[2];

        for (int i = 0; i < 2; i++) {
            std::getline(lineStream, segments[i], '=');
        }
        
        std::string lineElement;

        // Check if the string has a decimal point
        lineElement = segments[1];

        // Insert the key and value into a map as a named pair
        readResult.insert(std::make_pair(segments[0], lineElement));
    }

    return readResult;

}

void writeConfiguration(std::ostream &out, const ConfigMap &newMap) {
    // Write all the elements as key-value pairs
    for (const auto &pair : newMap) {
        out << pair.first << '=' << pair.second << std::endl;
    }

}

TrackerConfiguration *loadTrackerConfig() {
    static TrackerConfiguration conf;
    static bool loaded = false;

    if (loaded) {
        return &conf;
    }

    std::ifstream inFile;
    inFile.open(CONFIG_FILE_NAME);

    if (!inFile) {
        log("Config file not found. Creating...");
        writeTrackerConfig(conf);
        loaded = true;
        return &conf;
    }

    while (!inFile.eof()) {
        std::string wholeLine;
        getline(inFile, wholeLine);

        if (wholeLine[0] == '#') continue;

        std::stringstream lineStream(wholeLine);
        std::string segments[2];
        for (int i = 0; i < 2; i++) {
            getline(lineStream, segments[i], '=');
        }

        try {
            // Load in the configuration information
            if (segments[0] == "model") {
                conf.modelLocation = segments[1];
            } else if (segments[0] == "config") {
                conf.configLocation = segments[1];
            } else if (segments[0] == "confidence") {
                conf.confidence = stof(segments[1]);
            } else if (segments[0] == "skipFrames") {
                conf.skipFrames = stoi(segments[1]);
            } else if (segments[0] == "imageWidth") {
                conf.imageWidth = stoi(segments[1]);
            } else if (segments[0] == "imageHeight") {
                conf.imageHeight = stoi(segments[1]);
            } else if (segments[0] == "neuralNetQuality") {
                conf.neuralNetQuality = stoi(segments[1]);
            } else if (segments[0] == "useOpenCL") {
                conf.useOpenCL = bool(stoi(segments[1]));
            } else if (segments[0] == "useCSRT") {
                conf.useCSRT = bool(stoi(segments[1]));
            } else if (segments[0] == "maxDisappeared") {
                conf.maxDisappeared = stoi(segments[1]);
            } else if (segments[0] == "searchDistance") {
                conf.searchDistance = stoi(segments[1]);
            } else if (segments[0] == "useTracking") {
                conf.useTracking = bool(stoi(segments[1]));
            }

        } catch (std::invalid_argument &exception) {
            log("Invalid config entry. Continuing...");
            continue;
        }
    }

    loaded = true;
    return &conf;
}

void writeTrackerConfig(const TrackerConfiguration &conf) {
    std::ofstream outFile(CONFIG_FILE_NAME);

    outFile << "# BeeBit Configuration" << std::endl;
    outFile << "model=" << conf.modelLocation << std::endl;
    outFile << "config=" << conf.configLocation << std::endl;
    outFile << "confidence=" << conf.confidence << std::endl;
    outFile << "skipFrames=" << conf.skipFrames << std::endl;
    outFile << "imageWidth=" << conf.imageWidth << std::endl;
    outFile << "imageHeight=" << conf.imageHeight << std::endl;
    outFile << "useOpenCL=" << int(conf.useOpenCL) << std::endl;
    outFile << "useCSRT=" << int(conf.useCSRT) << std::endl;
    outFile << "neuralNetQuality=" << conf.neuralNetQuality << std::endl;
    outFile << "maxDisappeared=" << conf.maxDisappeared << std::endl;
    outFile << "searchDistance=" << conf.searchDistance << std::endl;
    outFile << "# Tracking is only supported on desktop platforms. Not IOT" << std::endl;
    outFile << "useTracking=" << conf.useTracking << std::endl;
    
    outFile << std::endl;

}

}