#pragma once

#include <stack>
#include "lexer.h"

namespace prism::ast {
    enum class NodeType { VARIABLE, INTEGER, FLOAT, ARRAY_ACCESS, OR, AND, IF };

    struct ASTNode {
        NodeType type;
        std::string value;
        int depth;
        bool root;

        std::shared_ptr<ASTNode> left, right;

        explicit ASTNode(NodeType t, std::string val = "", int depth = 0) : type(t), value(std::move(val)), depth(depth) {}
    };

    void print_ast_node(std::shared_ptr<prism::ast::ASTNode> node, int depth = 0);

    class Parser {
    private:
        std::vector<lexer::Token> tokens;
        size_t pos = 0;

    public:
        explicit Parser(std::vector<lexer::Token> tokenList) : tokens(std::move(tokenList)) {}
        std::shared_ptr<ASTNode> parse();
    private:
        std::shared_ptr<ASTNode> parseOr();
        std::shared_ptr<ASTNode> parseAnd();
        std::shared_ptr<ASTNode> parsePrimary();
        bool match(lexer::TokenType type);
        lexer::Token expect(lexer::TokenType type);
        lexer::Token previous();
    };
}