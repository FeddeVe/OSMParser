#pragma once
#include <string>
#include <fstream>

namespace FV{
    std::string getDirectoryFromPath(const std::string& fullPath) {
    size_t pos = fullPath.find_last_of("\\/");
    if (pos == std::string::npos) {
        return ""; // No directory part found
    }
    return fullPath.substr(0, pos);
}

  std::string getFileNameWithoutType(const std::string& fullPath) {
    size_t pos = fullPath.find_last_of(".");
    if (pos == std::string::npos) {
        return ""; // No directory part found
    }
    return fullPath.substr(0, pos);
}

bool fileExists(const std::string& filename) {
        std::ifstream file(filename);
        return file.good();
    }


std::string timeToString(uint64_t time)
{
    std::string timeString;
    uint64_t seconds = time % 60;
    uint64_t minutes = (time / 60) % 60;
    uint64_t hours = (time / 3600) % 24; 

    
    if (hours > 9)
    {
        timeString += std::to_string(hours) + ":";
            }else{
        timeString += "0"+ std::to_string(hours) + ":";
    }
    if (minutes > 9)
    {
        timeString += std::to_string(minutes) + ":";
    }else{
        timeString += "0"+ std::to_string(minutes) + ":";
    }
    if (seconds > 9)
    {
        timeString += std::to_string(seconds) + ":";
    }else{
        timeString += "0"+ std::to_string(seconds) + ":";
    }
    return timeString;
}
}