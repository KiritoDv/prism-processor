#pragma once

#include <stack>
#include <memory>
#include <variant>
#include "lexer.h"

namespace prism::ast {
    struct ASTNode;
    struct VariableNode { std::string name; };
    struct IntegerNode { int value; };
    struct FloatNode { float value; };
    struct ArrayAccessNode {
        std::shared_ptr<VariableNode> name;
        std::shared_ptr<std::vector<std::shared_ptr<ASTNode>>> arrayIndices;
    };
    struct OrNode { std::shared_ptr<ASTNode> left, right; };
    struct AndNode { std::shared_ptr<ASTNode> left, right; };
    struct ElseIfNode {
        std::shared_ptr<ASTNode> condition;
        std::shared_ptr<ASTNode> body;
    };
    struct IfNode {
        std::shared_ptr<ASTNode> condition;
        std::shared_ptr<ASTNode> body;
        std::shared_ptr<std::vector<std::shared_ptr<ElseIfNode>>> elseIfs;
        std::shared_ptr<ASTNode> elseBody;
    };
    struct EqualNode { std::shared_ptr<ASTNode> left, right; };
    struct RangeNode { std::shared_ptr<ASTNode> left, right; };
    struct InNode { std::shared_ptr<ASTNode> left, right; };
    struct NotNode { std::shared_ptr<ASTNode> node; };

    typedef std::variant<VariableNode, IntegerNode, FloatNode, ArrayAccessNode, OrNode, AndNode, IfNode, EqualNode, InNode, RangeNode, NotNode> ASTTypes;

    struct ASTNode {
        ASTTypes node;

        ASTNode(ASTTypes node) : node(std::move(node)) {}
    };

    void print_ast_node(std::shared_ptr<ASTNode> node, int depth = 0);

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
        std::shared_ptr<ASTNode> parseRange();
        std::shared_ptr<ASTNode> parseIn();
        std::shared_ptr<ASTNode> parsePrimary();
        bool match(lexer::TokenType type);
        lexer::Token expect(lexer::TokenType type);
        lexer::Token previous();
    };
}