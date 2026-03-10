#pragma once

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <functional>
#include <filesystem>

inline bool debugging = true;
inline bool ignoreWarnings = true;
inline std::ofstream file;

enum class LogType {
    Error,
    Warn,
    Info,
    Debug
};

// ANSI color codes for terminal output
inline const char* RESET  = "\033[0m";
inline const char* RED    = "\033[31m";
inline const char* YELLOW = "\033[33m";
inline const char* BLUE   = "\033[34m";
inline const char* GREEN  = "\033[32m";

inline std::function<void()> errorHandler;

inline void setupLogger() {
    file.open("logs.txt", std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << RED << "[Error] Could not open or create log file" << RESET << std::endl;
    }
}

inline void closeLogger() {
    if (file.is_open()) file.close();
}

// Get formatted timestamp like Gin: 2026/02/08 - 21:33:54
inline std::string currentTime() {
    std::time_t now = std::time(nullptr);
    std::tm tm_info;
    
    localtime_r(&now, &tm_info); // Linux/Unix safe

    char buffer[32]; // larger buffer
    std::strftime(buffer, sizeof(buffer), "%Y/%m/%d - %H:%M:%S", &tm_info);
    return std::string(buffer);
}

inline void log(LogType type, const std::string& message) {
    if (!debugging) return;

    std::string dt = currentTime();
    std::string output;
    std::string color;

    switch (type) {
        case LogType::Error:
            color = RED;
            output = "[ERROR] " + dt + " | " +  message;
            std::cerr << color << output << RESET << std::endl;
            if (file.is_open()) {
                file.flush();
                file << output << '\n';
            }
            if (errorHandler)
                errorHandler();
            break;
        case LogType::Warn:
            if (ignoreWarnings) return;
            color = YELLOW;
            output = "[WARN] " + dt + " | " +  message;
            std::cout << color << output << RESET << std::endl;
            if (file.is_open()) {
                file.flush();
                file << output << '\n';
            }
            break;
        case LogType::Info:
            output = "[INFO] " + dt + " | " +  message;
            std::cout << output <<  std::endl;
            if (file.is_open()) {
                file.flush();
                file << output << '\n';
            }
            break;
        case LogType::Debug:
            color = BLUE;
            output = "[DEBUG] " + dt + " | " +  message;
            std::cout << color << output << RESET << std::endl;
            if (file.is_open()) {
                file.flush();
                file << output << '\n';
            }
            break;
    }
}

// Optional shorthand for simple logging like Gin
inline void log(const std::string& message) {
    log(LogType::Info, message);
}

// Used for the message when a client connected
inline void logConnection(int status, const std::string& method, const std::string& endpoint) {
    if (!debugging) return;
    std::string dt = currentTime();
    std::string color;

    // Set color based on status code
    switch (status) {
        case 200: color = GREEN; break;
        case 404: color = YELLOW; break;
        case 500: color = RED; break;
        default:  color = RESET; break;
    }

    // Build a Gin style output
    std::ostringstream oss;
    oss << "[VESPER] - " << dt
        << " | " << color << status << RESET
        << " | " << method << " \"" << endpoint << "\"";

    std::string output = oss.str();

    // Print to console and file
    std::cout << output << std::endl;
    if (file.is_open()) {
        file << output << '\n';
        file.flush();
    }
}