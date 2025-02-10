#pragma once

#include <stack>
#include <memory>
#include <variant>
#include "lexer.h"

namespace prism::ast {
    struct VariableNode { std::string name; };
    struct IntegerNode { int value; };
    struct FloatNode { float value; };
    struct ArrayAccessNode {
        std::shared_ptr<VariableNode> name;
        std::shared_ptr<std::vector<std::shared_ptr<IntegerNode>>> arrayIndices;
    };
    struct OrNode { };
    struct AndNode { };
    struct IfNode {
        // condition = left, body = right
        std::shared_ptr<std::vector<std::shared_ptr<IfNode>>> elseIfs;
        std::shared_ptr<IfNode> elseBody;
    };
    struct EqualNode { };
    struct InNode { };

    struct ASTNode {
    public:
        ASTNode(std::variant<VariableNode, IntegerNode, FloatNode, ArrayAccessNode, OrNode, AndNode, IfNode, EqualNode, InNode> node) : node(std::move(node)) {}
        std::variant<VariableNode, IntegerNode, FloatNode, ArrayAccessNode, OrNode, AndNode, IfNode, EqualNode, InNode> node;
        std::shared_ptr<ASTNode> left, right;
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
        std::shared_ptr<ASTNode> parseEqual();
        std::shared_ptr<ASTNode> parseIn();
        std::shared_ptr<ASTNode> parsePrimary();
        bool match(lexer::TokenType type);
        lexer::Token expect(lexer::TokenType type);
        lexer::Token previous();
    };
}