#pragma once

#include <vector>
#include <string>
#include <variant>
#include <sstream>

#include "lexer.h"
#include "ast.h"

#define is_type(var, type) std::holds_alternative<type>((var))
#define M_ARRAY(arr, ...) prism::MTDArray { (uintptr_t) &arr, std::vector<size_t>(__VA_ARGS__) }

namespace prism {

    struct MTDArray {
        uintptr_t ptr;
        std::vector<size_t> dimensions;

        int& at(int x){
            return ((int*) ptr)[x];
        }

        int& at(int x, int y){
            return ((int*) ptr)[x * dimensions[1] + y];
        }

        int& at(int x, int y, int z){
            return ((int*) ptr)[x * dimensions[1] * dimensions[2] + y * dimensions[2] + z];
        }

        int& at(int x, int y, int z, int w){
            return ((int*) ptr)[x * dimensions[1] * dimensions[2] * dimensions[3] + y * dimensions[2] * dimensions[3] + z * dimensions[3] + w];
        }
    };

    typedef std::unordered_map<std::string, std::variant<bool, int, float, MTDArray>> ContextItems;

    enum class ScopeType {
        None,
        If,
        Else,
        ElseIf,
        For
    };

    enum class ExpressionType {
        None,
        Variable,
        If,
        Else,
        ElseIf,
        For,
        End
    };

    struct RuntimeContext {
        ScopeType scope = ScopeType::None;
        bool skipUntilEnd = false;
    };

    class Processor {
    public:
        void populate(ContextItems items);
        void load(const std::string& input);
        int evaluate(const std::shared_ptr<prism::ast::ASTNode>& node);
        std::string process();
    private:
        ContextItems m_items;
        RuntimeContext m_context;
        std::stringstream m_output;
        std::vector<std::string> m_lines;

        void evaluate_if(const std::string &line);
        void evaluate_for(const std::string &line);
    };
}