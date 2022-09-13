#ifndef UTILS_H
#define UTILS_H
#include <ctype.h>
#include <algorithm>
#include <string>
#include <set>
#include <sstream>

static bool isNewline(const char c) {
    return c == '\n' || c == '\r';
}
static bool isTab(const char c) {
    return c == '\t';
}
static bool isIDENFRChar(const char c) {
    return isalnum(c) || c == '_';
}
static bool isPunct(const char c) {
    static const std::set<char> punctSet = {';', ',', '(', ')', '[', ']', '{', '}', '+', '-', '*', '/',
                                            '%', '=', '<', '>', '<', '>', '=', '!', '!', '&', '|'};
    return punctSet.count(c);
};

static int stringToInt(const std::string& str) {
    std::stringstream ss;
    int i;
    ss << str;
    ss >> i;
    return i;
}

static void toUpper(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}
#endif