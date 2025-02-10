#include "ast.h"
#include "utils/exceptions.h"

std::shared_ptr<prism::ast::ASTNode> prism::ast::Parser::parse() {
    return parseOr();
}

std::shared_ptr<prism::ast::ASTNode> prism::ast::Parser::parseOr() {
    auto node = parseAnd();
    while (match(lexer::TokenType::OR)) {
        auto right = parseAnd();
        auto parent = std::make_shared<prism::ast::ASTNode>(NodeType::OR);
        parent->left = node;
        parent->right = right;
        node = parent;
    }
    return node;
}

std::shared_ptr<prism::ast::ASTNode> prism::ast::Parser::parseAnd() {
    auto node = parsePrimary();
    while (match(lexer::TokenType::AND)) {
        auto right = parsePrimary();
        auto parent = std::make_shared<prism::ast::ASTNode>(NodeType::AND);
        parent->left = node;
        parent->right = right;
        node = parent;
    }
    return node;
}

// Parses parentheses or variables
std::shared_ptr<prism::ast::ASTNode> prism::ast::Parser::parsePrimary() {
    if (match(lexer::TokenType::LPAREN)) {
        auto node = parseOr();
        expect(lexer::TokenType::RPAREN);  // Ensure closing ')'
        return node;
    }

    if (match(lexer::TokenType::INTEGER)) {
        return std::make_shared<ASTNode>(NodeType::INTEGER, previous().value);
    }

    if (match(lexer::TokenType::IN)) {
        int bp = 0;
        // TODO: Unimplemented
    }

    if (match(lexer::TokenType::IDENTIFIER)) {
        std::string varName = previous().value;
        std::shared_ptr<ASTNode> arrayNode = std::make_shared<ASTNode>(NodeType::VARIABLE, varName);

        int depth = 0;

        // Loop to handle multiple array accesses (e.g., var[0][1][2])
        while (match(lexer::TokenType::LBRACKET)) {
            auto indexNode = parsePrimary();  // Parse the index (e.g., 0, 1)
            expect(lexer::TokenType::RBRACKET);  // Expect closing bracket

            auto newArrayNode = std::make_shared<ASTNode>(NodeType::ARRAY_ACCESS, varName);
            newArrayNode->left = indexNode;
            newArrayNode->depth = ++depth;  // Track array depth
            newArrayNode->right = arrayNode;  // Create chain of accesses

            arrayNode = newArrayNode;  // Set this as the new root for next iteration
        }

        arrayNode->root = true;
        return arrayNode;
    }

    throw std::runtime_error("Unexpected token");
}

// Utility functions
bool prism::ast::Parser::match(lexer::TokenType type) {
    if (pos < tokens.size() && tokens[pos].type == type) {
        pos++;
        return true;
    }
    return false;
}

prism::lexer::Token prism::ast::Parser::expect(lexer::TokenType type) {
    if (pos < tokens.size() && tokens[pos].type == type) {
        return tokens[pos++];
    }

    throw prism::SyntaxError("Unexpected token");
}

prism::lexer::Token prism::ast::Parser::previous() { return tokens[pos - 1]; }