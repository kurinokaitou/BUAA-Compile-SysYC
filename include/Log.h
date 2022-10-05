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

static std::vector<ErrorLog> s_errorDump;
class Logger {
public:
    static void logError(ErrorType error, int lineNum, const std::string& meta = "", const std::string& target = "") {
        std::cout << "[error] line:" << lineNum << " " << genErrorText(error, meta, target) << std::endl;
        s_errorDump.emplace_back(lineNum, static_cast<std::underlying_type<ErrorType>::type>(error));
    }

    static void logError(const std::string& error) {
        std::cout << "[error] " << error << std::endl;
    }

    static void logInfo(const std::string& info) {
        std::cout << "[info] " << info << std::endl;
    }

    static void logDebug(const std::string& message) {
        std::cout << "[debug] " << message << std::endl;
    }
};

#endif