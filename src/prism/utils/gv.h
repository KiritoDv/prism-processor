#pragma once

#include <string>
#include <vector>
#include <optional>

namespace prism {
    namespace gv {
        std::vector<std::string> split(const std::string& line, const char delimiter);
        std::vector<std::string> parenthesis(const std::string& line);
        std::vector<std::string> fn_args(const std::string& line);
        std::optional<std::string> key_arg(const std::string& line, const std::string key);
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
}