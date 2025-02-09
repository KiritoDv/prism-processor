#pragma once

#include <vector>
#include <string>
#include <variant>

namespace prism {
    struct ContextItem {
        std::string key;
        std::variant<bool, int, float, std::string> value;
    };

    enum ScopeType {
        None,
        If,
        Else,
        ElseIf,
        For,
        End
    };

    struct RuntimeContext {
        ScopeType scope = ScopeType::None;
        bool multiline = false;
    };

    class Processor {
    public:
        void populate(std::vector<ContextItem> items);
        void load(const std::string& input);
        std::string process();
    private:
        std::vector<ContextItem> m_items;
        std::vector<std::string> m_lines;
        RuntimeContext m_context;
    };
}