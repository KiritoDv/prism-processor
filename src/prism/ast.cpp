#include "ast.h"
#include "utils/exceptions.h"

#define is_type(var, type) std::holds_alternative<type>((var))

std::shared_ptr<prism::ast::ASTNode> prism::ast::Parser::parse() {
    return parseOr();
}

std::shared_ptr<prism::ast::ASTNode> prism::ast::Parser::parseOr() {
    auto node = parseAnd();
    while (match(lexer::TokenType::OR)) {
        auto right = parseAnd();
        auto parent = std::make_shared<prism::ast::ASTNode>(OrNode{node, right});
        node = parent;
    }
    return node;
}

std::shared_ptr<prism::ast::ASTNode> prism::ast::Parser::parseAnd() {
    auto node = parseEqual();
    while (match(lexer::TokenType::AND)) {
        auto right = parseEqual();
        auto parent = std::make_shared<prism::ast::ASTNode>(AndNode{node, right});
        node = parent;
    }
    return node;
}

std::shared_ptr<prism::ast::ASTNode> prism::ast::Parser::parseEqual() {
    auto node = parseIn();
    while (match(lexer::TokenType::EQUAL)) {
        auto right = parseIn();
        auto parent = std::make_shared<prism::ast::ASTNode>(EqualNode{node, right});
        node = parent;
    }
    return node;
}

std::shared_ptr<prism::ast::ASTNode> prism::ast::Parser::parseIn() {
    auto node = parsePrimary();
    while (match(lexer::TokenType::IN)) {
        auto right = parsePrimary();
        auto parent = std::make_shared<prism::ast::ASTNode>(InNode{node, right});
        node = parent;
    }
    return node;
}
// Parses parentheses or variables
std::shared_ptr<prism::ast::ASTNode> prism::ast::Parser::parsePrimary() {
    if (match(lexer::TokenType::LPAREN)) {
        auto node = parse();
        expect(lexer::TokenType::RPAREN);  // Ensure closing ')'
        return node;
    }

    if (match(lexer::TokenType::INTEGER)) {
        return std::make_shared<ASTNode>(IntegerNode{std::stoi(previous().value)});
    }

    if (match(lexer::TokenType::IF)) {
        auto condition = parse();
        expect(lexer::TokenType::THEN);  // Ensure 'then' keyword
        auto body = parse();
        std::shared_ptr<std::vector<std::shared_ptr<ElseIfNode<ASTNode>>>> elseIfs = std::make_shared<std::vector<std::shared_ptr<ElseIfNode<ASTNode>>>>();
        while (match(lexer::TokenType::ELSEIF)) {
            auto elseIfCondition = parse();
            expect(lexer::TokenType::THEN);  // Ensure 'then' keyword
            auto elseIfBody = parse();
            elseIfs->push_back(std::make_shared<ElseIfNode<ASTNode>>(ElseIfNode<ASTNode>{elseIfCondition, elseIfBody}));
        }

        if (match(lexer::TokenType::ELSE)) {
            auto elseBody = parse();
            return std::make_shared<ASTNode>(IfNode<ASTNode>{condition, body, elseIfs, elseBody});
        }
        
        throw std::runtime_error("Unexpected token");
    }

    if (match(lexer::TokenType::NOT)) {
        int bp = 0;
        // TODO: Unimplemented
    }

    if (match(lexer::TokenType::IDENTIFIER)) {
        std::string varName = previous().value;
        std::shared_ptr<VariableNode> variable = std::make_shared<VariableNode>(VariableNode(varName));
        
        if (!match(lexer::TokenType::LBRACKET)) {
            return std::make_shared<ASTNode>(*variable);
        }

        std::shared_ptr<std::vector<std::shared_ptr<ASTNode>>> arrayIndices = std::make_shared<std::vector<std::shared_ptr<ASTNode>>>();

        // Loop to handle multiple array accesses (e.g., var[0][1][2])
        do {
            auto indexNode = parsePrimary();  // Parse the index (e.g., 0, 1)
            expect(lexer::TokenType::RBRACKET);  // Expect closing bracket

            arrayIndices->push_back(indexNode);
        } while (match(lexer::TokenType::LBRACKET));

        return std::make_shared<ASTNode>(ArrayAccessNode{variable, arrayIndices});
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

void prism::ast::print_ast_node(std::shared_ptr<prism::ast::ASTNode> node, int depth) {
    std::string indent = std::string(depth, ' ');
    if (is_type(node->node, VariableNode)) {
        std::cout << indent << "Variable: " << std::get<VariableNode>(node->node).name << std::endl;
        return;
    } else if (is_type(node->node, IntegerNode)) {
        std::cout << indent << "Integer: " << std::get<IntegerNode>(node->node).value << std::endl;
        return;
    } else if (is_type(node->node, FloatNode)) {
        std::cout << indent << "Float: " << std::get<FloatNode>(node->node).value << std::endl;
        return;
    } else if (is_type(node->node, ArrayAccessNode<ASTNode>)) {
        std::cout << indent << "Array Access: " << std::get<ArrayAccessNode<ASTNode>>(node->node).name->name << std::endl;
        for (auto& index : *std::get<ArrayAccessNode<ASTNode>>(node->node).arrayIndices) {
            print_ast_node(std::shared_ptr<ASTNode>(index.get()), depth + 1);
        }
        return;
    } else if (is_type(node->node, OrNode<ASTNode>)) {
        auto orNode = std::get<OrNode<ASTNode>>(node->node);
        std::cout << indent << "OR" << std::endl;
        print_ast_node(orNode.left, depth + 1);
        print_ast_node(orNode.right, depth + 1);
        return;
    } else if (is_type(node->node, AndNode<ASTNode>)) {
        auto andNode = std::get<AndNode<ASTNode>>(node->node);
        std::cout << indent << "AND" << std::endl;
        print_ast_node(andNode.left, depth + 1);
        print_ast_node(andNode.right, depth + 1);
        return;
    }
}