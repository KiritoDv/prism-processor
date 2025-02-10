#pragma once

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include "exceptions.h"

namespace prism::gv {
    std::vector<std::string> split(const std::string line, char delimiter);
    std::vector<std::string> parenthesis(const std::string line);
    std::vector<std::string> fn_args(const std::string line);
    std::unordered_map<std::string, std::optional<std::string>> il_args(const std::string line);
    template <typename T> T get(const std::string& arg) {
        // String
        if(arg[0] == '\'') {
            if(arg[arg.size() - 1] != '\'') {
                throw prism::SyntaxError("Invalid string");
            }

            return arg.substr(1, arg.size() - 2);
        }

        // Integer
        if(arg.find_first_not_of("0123456789") == std::string::npos) {
            return std::stoi(arg);
        }

        // Float
        if(arg.find_first_not_of("0123456789.") == std::string::npos) {
            return std::stof(arg);
        }

        // Boolean
        if(arg == "true" || arg == "false") {
            return arg == "true";
        }

        throw prism::SyntaxError("Unknown data type");
    }
}
