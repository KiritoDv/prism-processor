#include "processor.h"

#include <spdlog/spdlog.h>
#include <sstream>
#include <regex>

#include "utils/exceptions.h"
#include "utils/gv.h"

void prism::Processor::populate(std::vector<ContextItem> items) {
    m_items = items;
}

void prism::Processor::load(const std::string& data) {
    std::stringstream ss(data);
    std::string line;
    while (std::getline(ss, line)) {
        m_lines.push_back(line);
    }

    if(m_lines.empty()) {
        throw RuntimeError("No data to process");
    }

    if(m_lines[0].rfind("@prism", 0) != 0) {
        throw SyntaxError("Invalid prism file");
    }

    auto header = gv::parenthesis(m_lines[0].substr(6));
    auto args = gv::fn_args(header[0]);

    auto type = gv::key_arg(args[0], "type");
    if(!type.has_value()) {
        throw SyntaxError("Type not specified");
    }
    auto name = gv::key_arg(args[0], "name").value_or("prism_script");
    auto version = gv::key_arg(args[0], "version").value_or("1.0.0");
    auto description = gv::key_arg(args[0], "description").value_or("No description");
    auto author = gv::key_arg(args[0], "author").value_or("Unknown");
}

std::string prism::Processor::process() {
    std::stringstream ss;
    for (const auto& line : m_lines) {
        if (line.empty()) {
            continue;
        }
        
        if(line.rfind("@prism", 0) == 0) {
            continue;
        }


    }
}