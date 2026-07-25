#pragma once

namespace eyesee {

/** Log severity, from least to most urgent. */
enum class LogLevel { Debug, Info, Warn, Error };

/**
 * Static logging utility backed by Serial.
 *
 * Static rather than instance-based on purpose: logging is cross-cutting,
 * and threading a Logger reference through every constructor in the
 * firmware buys nothing. This is the one intentional exception to
 * "avoid globals" (see docs/architecture.md).
 */
class Logger {
public:
    static void init(unsigned long baudRate = 115200);

    static void debug(const char* tag, const char* message);
    static void info(const char* tag, const char* message);
    static void warn(const char* tag, const char* message);
    static void error(const char* tag, const char* message);

private:
    static void log(LogLevel level, const char* tag, const char* message);
    static const char* levelLabel(LogLevel level);
};

}  // namespace eyesee
