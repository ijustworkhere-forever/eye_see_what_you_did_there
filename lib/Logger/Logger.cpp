#include "Logger.h"

#include <Arduino.h>

namespace eyesee {

void Logger::init(unsigned long baudRate) {
    Serial.begin(baudRate);
}

void Logger::debug(const char* tag, const char* message) {
    log(LogLevel::Debug, tag, message);
}
void Logger::info(const char* tag, const char* message) {
    log(LogLevel::Info, tag, message);
}
void Logger::warn(const char* tag, const char* message) {
    log(LogLevel::Warn, tag, message);
}
void Logger::error(const char* tag, const char* message) {
    log(LogLevel::Error, tag, message);
}

void Logger::log(LogLevel level, const char* tag, const char* message) {
    Serial.print('[');
    Serial.print(levelLabel(level));
    Serial.print("] ");
    Serial.print(tag);
    Serial.print(": ");
    Serial.println(message);
}

const char* Logger::levelLabel(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warn: return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}

}  // namespace eyesee
