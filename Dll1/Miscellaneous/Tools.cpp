#include "Tools.hpp"

#include <sstream>
#include <filesystem>

std::vector<std::string> functionNames;

int count = 0;

std::filesystem::path localAppdata = getenv("LOCALAPPDATA");

std::filesystem::path logfile = localAppdata / "executelog.txt";

std::string gettime() {
    std::time_t rawTime;
    struct tm timeInfo;
    std::time(&rawTime);
    localtime_s(&timeInfo, &rawTime);
    char timeBuffer[19];
    std::sprintf(timeBuffer, "%02d-%02d_%02d-%02d-%02d",
        timeInfo.tm_mday,
        timeInfo.tm_mon + 1,
        timeInfo.tm_hour,
        timeInfo.tm_min,
        timeInfo.tm_sec);
    return std::string(timeBuffer);
}

bool debug = false; // <- set to true for debugging functions

void SetFunction(const std::string& funcName) {
    if (debug) {
        if (funcName == "DLLMain") {
            FILE* outFile = fopen(logfile.string().c_str(), "w");
            if (outFile)
                fclose(outFile);
        }

        if (functionNames.size() > 5000) {
            functionNames.clear();
            count += 1;
            std::stringstream ss;
            ss << "5000 x" << count;
            functionNames.push_back(ss.str());
        }
        functionNames.push_back(funcName);
        
        FILE* outFile = fopen(logfile.string().c_str(), "a");
        if (outFile != nullptr) {
            fprintf(outFile, "%s %s\n", gettime(), funcName.c_str());
            fclose(outFile);
        }
    }
}

std::string GetFunction() {
    std::string result;
    for (size_t i = 0; i < functionNames.size(); ++i) {
        result += "[" + std::to_string(i + 1) + "] " + functionNames[i] + "\n";
    }
    return result;
}