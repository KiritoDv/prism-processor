#include "prism/processor.h"

#include <spdlog/spdlog.h>
#include <fstream>

int main(int argc, char** argv) {
    if (argc < 2) {
        SPDLOG_ERROR("Usage: {} <file>", argv[0]);
        return 1;
    }

    std::ifstream input(argv[1]);
    if (!input.is_open()) {
        SPDLOG_ERROR("Failed to open file: {}", argv[1]);
        return 1;
    }

    std::vector<uint8_t> data = std::vector<uint8_t>(std::istreambuf_iterator(input), {});
    input.close();

    prism::Processor processor;
    processor.load(std::string(data.begin(), data.end()));
    SPDLOG_INFO("Processed data: {}", processor.process());
    return 0;
}