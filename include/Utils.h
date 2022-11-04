#ifndef UTILS_H
#define UTILS_H
#include <ctype.h>
#include <algorithm>
#include <string>
#include <set>
#include <sstream>
#include <vector>

static bool isNewline(const char c) {
    return c == '\n';
}
static bool isTab(const char c) {
    return c == '\t';
}
static bool isIdentChar(const char c) {
    return c == '_' || isalnum(c);
}
static bool isPunct(const char c) {
    static const std::set<char> punctSet = {';', ',', '(', ')', '[', ']', '{', '}', '+', '-', '*', '/',
                                            '%', '=', '<', '>', '<', '>', '=', '!', '!', '&', '|'};
    return punctSet.count(c);
};

static void toUpper(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

static void splitFormatString(const std::string& str, std::vector<std::string>& tokens, std::vector<bool>& place) {
    int i = 0;
    while (i < str.length()) {
        if (str[i] == '%' && str[i + 1] == 'd') {
            place.push_back(false);
            i += 2;
        } else {
            int lastPos = i;
            while (!(str[i] == '%' && str[i + 1] == 'd') && i < str.length()) {
                i++;
            }
            tokens.push_back(str.substr(lastPos, i - lastPos));
            place.push_back(true);
        }
    }
}

static std::size_t replaceAll(std::string& inout, const std::string& what, const std::string& with) {
    std::size_t count{};
    for (std::string::size_type pos{};
         inout.npos != (pos = inout.find(what.data(), pos, what.length()));
         pos += with.length(), ++count) {
        inout.replace(pos, what.length(), with.data(), with.length());
    }
    return count;
}
#endif