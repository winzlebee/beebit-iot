#include "bee_util.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace beebit {

static const std::string CONFIG_FILE_NAME = "beebit.cfg";

template<class T>
struct streamer {
    const T& val;
};
template<class T> streamer(T) -> streamer<T>;

template<class T>
std::ostream& operator<<(std::ostream& os, streamer<T> s) {
    os << s.val;
    return os;
}

template<class... Ts>
std::ostream& operator<<(std::ostream& os, streamer<std::variant<Ts...>> sv) {
   std::visit([&os](const auto& v) { os << streamer{v}; }, sv.val);
   return os;
}

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
        
        std::variant<int, float, std::string> lineElement;

        // Check if the string has a decimal point
        const bool isDecimal = segments[1].find('.') != std::string::npos;

        if (isDecimal) {
            lineElement = (float) atof(segments[1].c_str());
        } else {
            lineElement = atoi(segments[1].c_str());
            if (std::get<int>(lineElement) == 0) {
                // Store the value as a string
                lineElement = segments[1];
            }
        }

        // Insert the key and value into a map as a named pair
        readResult.insert(std::make_pair(segments[0], lineElement));
    }

    return readResult;

}

void writeConfiguration(const std::string &location, const ConfigMap &newMap) {
    std::ofstream outFile(location);

    if (!outFile) {
        return;
    }

    // Write all the elements as key-value pairs
    for (const auto &pair : newMap) {
        outFile << pair.first << '=' << streamer{pair.second} << std::endl;
    }

}

TrackerConfiguration *loadTrackerConfig() {
    static TrackerConfiguration conf;

    std::ifstream inFile;
    inFile.open(CONFIG_FILE_NAME);

    if (!inFile) {
        log("Config file not found. Creating...");
        writeTrackerConfig(conf);
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
            } else if (segments[0] == "raspi") {
                conf.usePiCam = bool(stoi(segments[1]));
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
            }

        } catch (std::invalid_argument &exception) {
            log("Invalid config entry. Continuing...");
            continue;
        }
    }

    return &conf;
}

void writeTrackerConfig(const TrackerConfiguration &conf) {
    std::ofstream outFile(CONFIG_FILE_NAME);

    outFile << "# BeeBit Configuration" << std::endl;
    outFile << "model=" << conf.modelLocation << std::endl;
    outFile << "config=" << conf.configLocation << std::endl;
    outFile << "confidence=" << conf.confidence << std::endl;
    outFile << "skipFrames=" << conf.skipFrames << std::endl;
    outFile << "raspi=" << conf.usePiCam << std::endl;
    outFile << "imageWidth=" << conf.imageWidth << std::endl;
    outFile << "imageHeight=" << conf.imageHeight << std::endl;
    outFile << "useOpenCL=" << int(conf.useOpenCL) << std::endl;
    outFile << "useCSRT=" << int(conf.useCSRT) << std::endl;
    outFile << "neuralNetQuality=" << conf.neuralNetQuality << std::endl;
    outFile << "maxDisappeared=" << conf.maxDisappeared << std::endl;
    outFile << "searchDistance=" << conf.searchDistance << std::endl;
    
    outFile << std::endl;

}

}