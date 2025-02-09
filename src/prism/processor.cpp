#include "processor.h"

#include <spdlog/spdlog.h>
#include <sstream>

void prism::Processor::populate(std::vector<ContextItem> items) {
    m_items = items;
}

void prism::Processor::load(std::string data) {
    std::stringstream ss(data);
    std::string line;
    while (std::getline(ss, line)) {
        m_lines.push_back(line);
    }
}

std::string prism::Processor::process() {
    std::stringstream ss;
    for (const auto& line : m_lines) {

    }
}