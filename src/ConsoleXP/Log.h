#pragma once

#include <windows.h>
#include <fstream>
#include <mutex>
#include <cstdarg>
#include <cstdio>
#include <string>
#include <ctime>
#include <sstream>

namespace Log {
    inline std::mutex& GetMutex() {
        static std::mutex logMutex;
        return logMutex;
    }

    inline const char* GetLogFilePath() {
        return "log-cxp.txt";
    }

    inline void Initialize() {
        std::lock_guard<std::mutex> lock(GetMutex());
        std::ofstream logFile(GetLogFilePath(), std::ios::out); // Overwrite mode
        if (logFile.is_open()) {
            logFile << "===== ConsoleXP Log Started =====" << std::endl;
        }
    }

    inline std::string GetCurrentTimestamp() {
        std::time_t now = std::time(nullptr);
        std::tm localTime;
        localtime_s(&localTime, &now);

        char buffer[64];
        std::strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S]", &localTime);
        return std::string(buffer);
    }

    inline void Write(const char* fmt, ...) {
        std::lock_guard<std::mutex> lock(GetMutex());

        char messageBuffer[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf_s(messageBuffer, sizeof(messageBuffer), _TRUNCATE, fmt, args);
        va_end(args);

        std::ofstream logFile(GetLogFilePath(), std::ios::app);
        if (logFile.is_open()) {
            logFile << GetCurrentTimestamp() << " " << messageBuffer << std::endl;
        }
        else {
            std::string debugMessage = GetCurrentTimestamp() + " " + messageBuffer;
            OutputDebugStringA(debugMessage.c_str());
            OutputDebugStringA("\n");
        }
    }

    inline void Write(const std::string& message) {
        Write("%s", message.c_str());
    }
}
