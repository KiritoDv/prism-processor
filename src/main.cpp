#include "prism/processor.h"

#include <spdlog/spdlog.h>
#include <fstream>

prism::ContextTypes add_text(std::vector<prism::ContextTypes> args) {
    std::string items = "";
    for (int i = 0; i<args.size(); i++) {
        items += "add";
    }
    return prism::ContextTypes{items};
}

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
    float o_float[] = { 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f };
    int o_masks[2] = { 1, 1 };
    int o_blend[2] = { 1, 1 };
    int o_c[2][2][3] = { { { 1, 1, 1 }, { 1, 1, 1 } }, { { 1, 1, 1 }, { 1, 1, 1 } } };

    prism::ContextItems vars = {
        VAR( "o_textures", M_ARRAY(o_textures, int, 2) ),
        VAR("o_clamp", M_ARRAY(o_clamp, int, 2, 2)),
        VAR("o_float", M_ARRAY(o_float, float, 6)),
        VAR("o_fog", true) ,
        VAR("o_grayscale", true),
        VAR("o_inputs", 4),
        VAR("o_alpha", true),
        VAR("o_masks", M_ARRAY(o_masks, int, 2)),
        VAR("o_blend", M_ARRAY(o_blend, int, 2)),
        VAR("o_three_point_filter", true),
        VAR("o_2cyc", true),
        VAR("o_c", M_ARRAY(o_c, int, 3, 2, 2)),
        VAR("SHADER_COMBINED", 1),
        VAR("add_text", add_text),
    };

    prism::Processor processor;
    processor.populate(vars);
    processor.load(std::string(data.begin(), data.end()));
    SPDLOG_INFO("Processed data: \n{}", processor.process());
    return 0;
}