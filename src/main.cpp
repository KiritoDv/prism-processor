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

    int o_textures[2] = { 1, 1 };
    int o_clamp[2][2] = { { 1, 1, }, { 1, 1 } };

    prism::ContextItems vars = {
        { "o_textures", M_ARRAY(o_textures, int) },
        { "o_clamp", M_ARRAY(o_clamp, int, 2, 2) }
    };

    prism::Processor processor;
    processor.populate(vars);
    processor.load(std::string(data.begin(), data.end()));
    SPDLOG_INFO("Processed data: \n{}", processor.process());
    return 0;
}