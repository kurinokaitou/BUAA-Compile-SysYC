#include <vector>
#include <string>
#include <iostream>
static void splitString(const std::string& str, std::vector<std::string>& tokens, std::vector<bool>& place, const std::string& delimiters) {
    int i = 0;
    while (i < str.size()) {
        if (str[i] == '%' && str[i + 1] == 'd') {
            place.push_back(false);
            i += 2;
        } else {
            int lastPos = i;
            while (!(str[i] == '%' && str[i + 1] == 'd')) {
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

int main() {
    std::string str("woain\ndsjan\n");
    replaceAll(str, "\n", "\\0A");

    std::cout << str << std::endl;
    return 0;
}