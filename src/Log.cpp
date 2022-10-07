#include <Log.h>

#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */

std::vector<ErrorLog> Logger::s_errorDump;
void Logger::logError(ErrorType error, int lineNum, const std::string& meta, const std::string& target) {
    std::cout << BOLDRED << "[error]" << RESET
              << " line:" << lineNum << " " << genErrorText(error, meta, target) << std::endl;
    s_errorDump.emplace_back(lineNum, static_cast<std::underlying_type<ErrorType>::type>(error));
}

void Logger::logError(const std::string& error) {
    std::cout << BOLDRED << "[error]" << RESET << " " << error << std::endl;
}

void Logger::logInfo(const std::string& info) {
    std::cout << BOLDBLUE << "[info]" << RESET << " " << info << std::endl;
}

void Logger::logDebug(const std::string& message) {
    std::cout << BOLDYELLOW << "[debug]" << RESET << " " << message << std::endl;
}
