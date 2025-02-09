#include "gv.h"

#include <regex>
#include <sstream>
#include "utils/exceptions.h"

std::vector<std::string> prism::gv::split(const std::string& line, const char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        result.push_back(item);
    }
    return result;
}

std::vector<std::string> prism::gv::parenthesis(const std::string& line) {
    std::vector<std::string> result;
    std::regex re(R"(\(([^)]+)\))");
    std::sregex_iterator next(line.begin(), line.end(), re);
    std::sregex_iterator end;
    while (next != end) {
        std::smatch match = *next;
        result.push_back(match[1].str());
        next++;
    }
    return result;
}

std::vector<std::string> prism::gv::fn_args(const std::string& line) {
    std::vector<std::string> result;
    std::regex re(R"(\s*,\s*)");
    std::sregex_token_iterator first{line.begin(), line.end(), re, -1}, last;

    std::copy_if(first, last, std::back_inserter(result), 
                 [](const std::string& s) { return !s.empty(); });
    return result;
}

std::optional<std::string> prism::gv::key_arg(const std::string& line, const std::string key) {
    std::regex re(key + R"(=([^,]+))");
    std::smatch match;
    if (std::regex_search(line, match, re)) {
        return match[1].str();
    }

    return std::nullopt;
}