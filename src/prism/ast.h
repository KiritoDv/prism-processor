#pragma once

#include <stack>
#include <memory>
#include <variant>
#include "lexer.h"

namespace prism::ast {
    struct VariableNode { std::string name; };
    struct IntegerNode { int value; };
    struct FloatNode { float value; };
    template<typename T> struct ArrayAccessNode {
        std::shared_ptr<VariableNode> name;
        std::shared_ptr<std::vector<std::shared_ptr<T>>> arrayIndices;
    };
    template<typename T> struct OrNode { std::shared_ptr<T> left, right; };
    template<typename T> struct AndNode { std::shared_ptr<T> left, right; };
    template<typename T> struct ElseIfNode {
        std::shared_ptr<T> condition;
        std::shared_ptr<T> body;
    };
    template<typename T> struct IfNode {
        std::shared_ptr<T> condition;
        std::shared_ptr<T> body;
        std::shared_ptr<std::vector<std::shared_ptr<ElseIfNode<T>>>> elseIfs;
        std::shared_ptr<T> elseBody;
    };
    template<typename T> struct EqualNode { std::shared_ptr<T> left, right; };
    template<typename T> struct InNode { std::shared_ptr<T> left, right; };

    struct ASTNode {
        std::variant<VariableNode, IntegerNode, FloatNode, ArrayAccessNode<ASTNode>, OrNode<ASTNode>, AndNode<ASTNode>, IfNode<ASTNode>, EqualNode<ASTNode>, InNode<ASTNode>> node;
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