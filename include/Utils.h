#ifndef UTILS_H
#define UTILS_H
#include <cctype>

static bool isNewline(const char& c) {
    return c == '\n' || c == '\r';
}
static bool isTab(const char& c) {
    return c == '\t';
}
#endif