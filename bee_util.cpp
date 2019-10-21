#include "bee_util.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

namespace beebit {

static const std::string CONFIG_FILE_NAME = "beebit.cfg";

ConfigMap readConfiguration(std::istream &stream, const char delim) {
    ConfigMap readResult;

    if (!stream) {
        return readResult;
    }

    // Read the lines of the config into the configuration map
    std::string currentLine;
    while (std::getline(stream, currentLine, delim)) {
        if (currentLine.at(0) == '#') continue;

        // For the current line, separate the configuration into a set of keys and values
        std::stringstream lineStream(currentLine);
        std::string segments[2];

        for (int i = 0; i < 2; i++) {
            std::getline(lineStream, segments[i], '=');
        }

        // Insert the key and value into a map as a named pair
        readResult.insert(std::make_pair(segments[0], segments[1]));
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

    ConfigMap config = readConfiguration(inFile, '\n');

    for (const auto pair : config) {
        try {
            // Load in the configuration information
            if (pair.first == "model") {
                conf.modelLocation = pair.second;
            } else if (pair.first == "config") {
                conf.configLocation = pair.second;
            } else if (pair.first == "confidence") {
                conf.confidence = stof(pair.second);
            } else if (pair.first == "skipFrames") {
                conf.skipFrames = stoi(pair.second);
            } else if (pair.first == "imageWidth") {
                conf.imageWidth = stoi(pair.second);
            } else if (pair.first == "imageHeight") {
                conf.imageHeight = stoi(pair.second);
            } else if (pair.first == "neuralNetQuality") {
                conf.neuralNetQuality = stoi(pair.second);
            } else if (pair.first == "useOpenCL") {
                conf.useOpenCL = bool(stoi(pair.second));
            } else if (pair.first == "useCSRT") {
                conf.useCSRT = bool(stoi(pair.second));
            } else if (pair.first == "maxDisappeared") {
                conf.maxDisappeared = stoi(pair.second);
            } else if (pair.first == "searchDistance") {
                conf.searchDistance = stoi(pair.second);
            } else if (pair.first == "useTracking") {
                conf.useTracking = bool(stoi(pair.second));
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