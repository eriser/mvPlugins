#pragma once

namespace mvSynth {

enum class LogType
{
    Debug,
    Info,
    Warning,
    Error
};

void Log(LogType type, const char* file, int line, const char* message);

#define LOG_DEBUG(message)   Log(LogType::Debug, __FILE__, __LINE__, (message))
#define LOG_INFO(message)    Log(LogType::Info, __FILE__, __LINE__, (message))
#define LOG_WARNING(message) Log(LogType::Warning, __FILE__, __LINE__, (message))
#define LOG_EROOR(message)   Log(LogType::Error, __FILE__, __LINE__, (message))

} // namespace mvSynth