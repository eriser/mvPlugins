#include "stdafx.h"
#include "Logger.h"

namespace mvSynth {

void Log(LogType type, const char* file, int line, const char* message)
{
    std::string str;
    str += file;
    str += '(';
    str += std::to_string(line);
    str += ") [";

    switch (type)
    {
    case LogType::Info:
        str += "INFO";
        break;
    case LogType::Debug:
        str += "DEBUG";
        break;
    case LogType::Warning:
        str += "WARNING";
        break;
    case LogType::Error:
        str += "ERROR";
        break;
    }

    str += "] ";
    str += message;
    str += '\n';

    OutputDebugStringA(str.c_str());
}

} // namespace mvSynth