#pragma once

#include <iostream>
#include <chrono>
#include <string>
#include <cstdarg>
#include <cstdio>

// This class measures time from its creation to its destruction (RAII)
class ScopedTimer {
public:
    ScopedTimer(std::string name) 
        : name_(std::move(name)), start_(std::chrono::high_resolution_clock::now()) {}

    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_).count();
        
        // Lowered threshold to 1us because Phase 3 is incredibly fast.
        // If it's 0, it will print EVERY request (use 0 only for heavy debugging).
        if (duration >= 1) { 
             std::printf(" [%s]  [TRACE] %-25s | %4ld us\n",__DATE__, name_.c_str(), duration);
             std::printf("---------------------------------------------------\n");
        }
    }

private:
    std::string name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

// Macro to create a unique timer variable for each line
#define TRACE_EVENT(name) ScopedTimer timer##__LINE__(name)

enum class LogLevel { INFO, WARN, ERROR, DEBUG };

inline void DB_LOG(LogLevel level, const char* fmt, ...) {
    const char* level_str = "INFO";
    if (level == LogLevel::ERROR) level_str = "ERROR";
    else if (level == LogLevel::WARN) level_str = "WARN";
    else if (level == LogLevel::DEBUG) level_str = "DEBUG";
    
    va_list args;
    va_start(args, fmt);
    std::printf("[%s] ", level_str);
    std::vprintf(fmt, args);
    std::printf("\n");
    va_end(args);
}
