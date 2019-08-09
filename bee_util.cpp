#include "bee_util.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace beebit {

static const std::string CONFIG_FILE_NAME = "beebit.cfg";

Configuration *loadConfig() {
    static Configuration conf;

    std::ifstream inFile;
    inFile.open(CONFIG_FILE_NAME);

    if (!inFile) {
        log("Config file not found. Creating...");
        writeConfig(conf);
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
            }

        } catch (std::invalid_argument &exception) {
            log("Invalid config entry. Continuing...");
            continue;
        }
    }

    return &conf;
}

void writeConfig(const Configuration &conf) {
    std::ofstream outFile(CONFIG_FILE_NAME);

    outFile << "# BeeBit Configuration" << std::endl;
    outFile << "model=" << conf.modelLocation << std::endl;
    outFile << "config=" << conf.configLocation << std::endl;
    outFile << "confidence=" << conf.confidence << std::endl;
    outFile << "skipFrames=" << conf.skipFrames << std::endl;
    outFile << "raspi=" << conf.usePiCam << std::endl;
    
    outFile << std::endl;

}

void log(const std::string &text) {
    std::cout << "BeeBit: " << text << std::endl;
}

}