
#include "string.hpp"

namespace base {

std::vector<std::string> split_string(const std::string& s, const std::string& sep) {
    std::vector<std::string> tokens;
    
    if (sep.empty()) {
        tokens.push_back(s);
        return tokens;
    }

    size_t start = 0;
    size_t end = s.find(sep);

    while (end != std::string::npos) {
        tokens.emplace_back(s.substr(start, end - start));
        start = end + sep.length();
        end = s.find(sep, start);
    }

    tokens.emplace_back(s.substr(start));

    return tokens;
}

}
