#pragma once

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>

inline bool debugging = true;
inline bool timeDebugging = true;
inline std::ofstream file;

enum class LogType {
    Error,
    Warn,
    Info,
    Debug
};

inline void setupLogger() { // Runs in TcpServer constructor
    file.open("logs.txt");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file." << std::endl;
        return;
    }
}

inline void closeLogger() { // Runs in TcpServer destructor
    file.close();
}

inline void log(std::string message) {
    if (!debugging) return;
    
    std::string dt = "";
    if (timeDebugging) {
        time_t now = time(nullptr);
        dt = ctime(&now);
        dt.pop_back();
    }
    std::cout << dt << " " << message << '\n';
    file << dt << " " << message << '\n';
    file.flush();
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
            file << dt << " [ERROR] " << message << '\n';
            file.flush();
            exit(1);
        case LogType::Info:
            std::cout << dt << " [INFO] " << message << '\n';
            file << dt << " [INFO] " << message << '\n';
            file.flush();
            break;
        case LogType::Warn:
            std::cout << dt << " [WARN] " << message << '\n';
            file << dt << " [WARN] " << message << '\n';
            file.flush();
            break;
        case LogType::Debug:
            std::cout << dt << " [DEBUG] " << message << '\n';
            file << dt << " [DEBUG] " << message << '\n';
            file.flush();
            break;
    }
}