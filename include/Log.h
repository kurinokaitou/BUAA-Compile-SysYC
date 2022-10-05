#ifndef LOG_H
#define LOG_H
#include <vector>
#include <iostream>
#include <Error.h>

#ifndef DEBUG
#define DBG_ERROR(...) throw std::runtime_error(__VA_ARGS__);
#define DBG_LOG(...) Logger::logDebug(__VA_ARGS__);
#else
#define DBG_ERROR(...)
#define DBG_LOG(...)
#endif

struct ErrorLog {
    int lineNum;
    char code;
    ErrorLog(int i, char c) :
        lineNum(i), code(c) {}
    friend std::ostream& operator<<(std::ostream& os, const ErrorLog& error) {
        os << error.lineNum << " " << error.code << "\n";
        return os;
    }
};

class Logger {
public:
    static void logError(ErrorType error, int lineNum, const std::string& meta = "", const std::string& target = "");
    static void logError(const std::string& error);
    static void logInfo(const std::string& info);
    static void logDebug(const std::string& message);
    static std::vector<ErrorLog> s_errorDump;
};

#endif