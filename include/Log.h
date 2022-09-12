#ifndef LOG_H
#define LOG_H
#include <string>
#define PARSER_LOG_ERROR(...)                  \
    do {                                       \
        Logger::logError(__VA_ARGS__);         \
        throw std::runtime_error(__VA_ARGS__); \
    } while (false)

class Logger {
public:
    static void logError(const std::string& error) {
        // std::cout << error << std::endl;
        // TODO: 将错误记录入文件中
    }
};
#endif