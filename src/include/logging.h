#pragma once

#include <iostream>
#include <cstdlib>
#include <ctime>

inline bool debugging = true;
inline bool timeDebugging = true;

enum class LogType {
    Error,
    Warn,
    Info,
    Debug
};

inline void log(std::string message) {
    if (!debugging) return;
    
    std::string dt = "";
    if (timeDebugging) {
        time_t now = time(nullptr);
        dt = ctime(&now);
        dt.pop_back();
    }
    std::cout << dt << " " << message << '\n';
}

inline void log(LogType logType, std::string message) {
    if (!debugging) return;

    std::string dt = "";
    if (timeDebugging) {
        time_t now = time(nullptr);
        dt = ctime(&now);
        dt.pop_back();
    }
    switch (logType) {
        case LogType::Error:
            std::cerr << dt << " [ERROR] " << message << '\n';
            exit(1);
        case LogType::Info:
            std::cout << dt << " [INFO] " << message << '\n';
            break;
        case LogType::Warn:
            std::cout << dt << " [WARN] " << message << '\n';
            break;
        case LogType::Debug:
            std::cout << dt << " [DEBUG] " << message << '\n';
            break;
    }
}