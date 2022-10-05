#include <Log.h>
std::vector<ErrorLog> Logger::s_errorDump;
void Logger::logError(ErrorType error, int lineNum, const std::string& meta, const std::string& target) {
    std::cout << "[error] line:" << lineNum << " " << genErrorText(error, meta, target) << std::endl;
    s_errorDump.emplace_back(lineNum, static_cast<std::underlying_type<ErrorType>::type>(error));
}

void Logger::logError(const std::string& error) {
    std::cout << "[error] " << error << std::endl;
}

void Logger::logInfo(const std::string& info) {
    std::cout << "[info] " << info << std::endl;
}

void Logger::logDebug(const std::string& message) {
    std::cout << "[debug] " << message << std::endl;
}
