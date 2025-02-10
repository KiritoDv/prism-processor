#include "processor.h"

#include <spdlog/spdlog.h>
#include <sstream>
#include <regex>

#include "utils/exceptions.h"
#include "utils/gv.h"

void prism::Processor::populate(ContextItems items) {
    m_items = items;
}

void prism::Processor::load(const std::string& data) {
    std::stringstream ss(data);
    std::string line;
    while (std::getline(ss, line)) {
        m_lines.push_back(line);
    }

    if(m_lines.empty()) {
        throw RuntimeError("No data to process");
    }

    if(m_lines[0].rfind("@prism", 0) != 0) {
        throw SyntaxError("Invalid prism file");
    }

    auto header = gv::parenthesis(m_lines[0].substr(6));
    auto args = gv::il_args(header[0]);

    if(!args.contains("type")){
        throw SyntaxError("Type not specified");
    }

    auto type = args["type"];
    auto name = args["name"].value_or("prism_script");
    auto version = args["version"].value_or("1.0.0");
    auto description = args["description"].value_or("Unknown");
    auto author = args["author"].value_or("Someone very clever");
    m_lines.erase(m_lines.begin());
    if(m_lines[0].empty()) {
        m_lines.erase(m_lines.begin());
    }
    m_input = "";
    for(const auto& line : m_lines){
        m_input += line + "\n";
    }
    SPDLOG_INFO("Prism script: {} v{} by {}", name, version, author);
}

std::string trim(const std::string& str) {
    auto start = std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    });

    auto end = std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base();

    return (start < end) ? std::string(start, end) : "";
}

prism::ExpressionType ev_expr(const std::string& raw){
    const auto line = trim(raw);

    if(line.rfind("@if", 0) == 0){
        return prism::ExpressionType::If;
    }

    if(line.rfind("@else", 0) == 0){
        return prism::ExpressionType::Else;
    }

    if(line.rfind("@elseif", 0) == 0){
        return prism::ExpressionType::ElseIf;
    }

    if(line.rfind("@for", 0) == 0){
        return prism::ExpressionType::For;
    }

    if(line.rfind("@end", 0) == 0){
        return prism::ExpressionType::End;
    }

    if(line[0] == '@'){
        return prism::ExpressionType::Variable;
    }

    return prism::ExpressionType::None;
}

int prism::Processor::evaluate(const std::shared_ptr<prism::ast::ASTNode>& node) {
    if (!node) return false;
    switch (node->type) {
        case ast::NodeType::VARIABLE: {
            if(!this->m_items.contains(node->value)){
                throw SyntaxError("Unknown variable " + node->value);
            }
            auto var = this->m_items.at(node->value);
            if(is_type(var, int)){
                return std::get<int>(var);
            }
            if(is_type(var, bool)){
                return std::get<bool>(var);
            }
            throw SyntaxError("Invalid variable type on if");
        }
        case ast::NodeType::INTEGER:
            return std::stoi(node->value);
        case ast::NodeType::ARRAY_ACCESS: {
            if(!node->root) {
                return evaluate(node->left);
            }

            if(!this->m_items.contains(node->value)){
                throw SyntaxError("Unknown variable " + node->value);
            }

            auto var = this->m_items.at(node->value);

            if(!is_type(var, MTDArray)) {
                throw SyntaxError(node->value + " is not an array");
            }

            auto array = std::get<MTDArray>(var);

            switch (node->depth) {
                case 1:
                    return array.at(evaluate(node->left));
                case 2:
                    return array.at(evaluate(node->right),
                                    evaluate(node->left));
                case 3:
                    return array.at(evaluate(node->right->right->left), evaluate(node->right->left),
                                    evaluate(node->left));
                case 4:
                    return array.at(evaluate(node->right->right->right->left), evaluate(node->right->right->left),
                                    evaluate(node->right->left), evaluate(node->left));
            }

            throw SyntaxError("We dont support array indexes bigger than 4");
        }
        case ast::NodeType::OR:
            return evaluate(node->left) == 1 || evaluate(node->right) == 1;
        case prism::ast::NodeType::AND:
            return evaluate(node->left) == 1 && evaluate(node->right) == 1;
        case ast::NodeType::FLOAT:
            break;
    }
    return false;
}

void prism::Processor::evaluate_if(const std::string& line) {
    m_context.scope = ScopeType::If;
    m_context.skipUntilEnd = false;

    size_t pos = 3;

    auto raw = gv::parenthesis(line.substr(pos));
    lexer::Lexer eval(raw[0]);
    auto tokens = eval.tokenize();
    ast::Parser parser(tokens);
    auto ast = parser.parse();
    auto result = evaluate(ast) == 1;

    pos += eval.length() + 2;
    if(result){
        if(line.length() > pos){
            m_output << trim(line.substr(pos, line.length() - pos)) << "\n";
            m_context.scope = ScopeType::None;
        }
    } else {
        m_context.skipUntilEnd = true;
    }
}

void prism::Processor::evaluate_for(const std::string& line) {
    m_context.scope = ScopeType::For;
    m_context.skipUntilEnd = false;

    size_t pos = 4;

    auto raw = gv::parenthesis(line.substr(pos));
    lexer::Lexer eval(raw[0]);
    auto tokens = eval.tokenize();
    ast::Parser parser(tokens);
    auto ast = parser.parse();
    auto result = evaluate(ast) == 1;

    pos += eval.length() + 2;
    if(result){
        if(line.length() > pos){
            m_output << trim(line.substr(pos, line.length() - pos)) << "\n";
            m_context.scope = ScopeType::None;
        }
    } else {
        m_context.skipUntilEnd = true;
    }
}

std::string get_parenthesis(std::string::iterator& c, std::string::iterator end){
    auto start = c;
    int parenthesis = 0;
    while (c != end) {
        if (*c == '(') {
            parenthesis++;
        }
        if (*c == ')') {
            parenthesis--;
        }
        if (parenthesis == 0) {
            break;
        }
        c++;
    }
    if (c == end) {
        throw prism::SyntaxError("Unterminated parenthesis");
    }
    c++;
    return std::string(start, c);
}

std::shared_ptr<prism::ast::ASTNode> parse_parentesis(std::string::iterator& c, std::string::iterator end){
    prism::lexer::Lexer eval(get_parenthesis(c, end));
    auto tokens = eval.tokenize();
    prism::ast::Parser parser(tokens);
    auto ast = parser.parse();
    return ast;
}

std::string get_accolade(std::string::iterator& c, std::string::iterator end){
    auto start = c+1;
    int accolade = 0;
    while (c != end) {
        if (*c == '{') {
            accolade++;
        }
        if (*c == '}') {
            accolade--;
        }
        if (accolade == 0) {
            break;
        }
        c++;
    }
    if (c == end) {
        throw prism::SyntaxError("Unterminated accolade");
    }
    auto result = std::string(start, c);
    c++;
    return result;
}

std::shared_ptr<prism::ast::ASTNode> parse_accolade(std::string::iterator& c, std::string::iterator end){
    prism::lexer::Lexer eval(get_accolade(c, end));
    auto tokens = eval.tokenize();
    prism::ast::Parser parser(tokens);
    auto ast = parser.parse();
    return ast;
}

std::string get_keyword(std::string::iterator& c, std::string::iterator end){
    auto start = c;
    char match[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '_'};
    while (std::find(std::begin(match), std::end(match), *c) != std::end(match) && c != end) {
        c++;
    }
    return std::string(start, c);
}

prism::Node parse(std::string input) {
    std::shared_ptr<prism::Node> root = std::make_shared<prism::Node>(prism::TextNode(""), nullptr);
    std::shared_ptr<prism::Node> current = root;
    auto c = input.begin();
    auto previous = c;
    while (c != input.end()) {
        if (*c == '@') {
            current->children.push_back(std::make_shared<prism::Node>(prism::TextNode(std::string(previous, c)), current));
            c++;
            if (*c == '{') {
                auto ast = parse_accolade(c, input.end());
                // get_accolade(c, input.end());
                // auto ast = nullptr;
                current->children.push_back(std::make_shared<prism::Node>(prism::VariableNode(ast), current));
            }
            auto expr = get_keyword(c, input.end());
            previous = c;
            if (expr == "if") {
                auto ast = parse_parentesis(c, input.end());
                // get_parenthesis(c, input.end());
                // auto ast = nullptr;
                current->children.push_back(std::make_shared<prism::Node>(prism::IfNode(ast), current));

                current = current->children.back();
                previous = c;

                bool sameLine = false;
                while(*c != '\n'){
                    if (*c == ';') {
                        sameLine = true;
                        break;
                    }
                    c++;
                }
                if(sameLine){
                    while (*c != '\n') {
                        c++;
                    }
                    current->children.push_back(std::make_shared<prism::Node>(prism::TextNode(std::string(previous, c)), current));
                    previous = c;
                    current = current->parent;
                }
            } else if (expr == "else") {
                current = current->parent;
                previous = c;
                current->children.push_back(std::make_shared<prism::Node>(prism::ElseNode(), current));
                current = current->children.back();
                previous = c;
                bool sameLine = false;
                while(*c != '\n'){
                    if (*c == ';') {
                        sameLine = true;
                        break;
                    }
                    c++;
                }
                if(sameLine){
                    while (*c != '\n') {
                        c++;
                    }
                    current->children.push_back(std::make_shared<prism::Node>(prism::TextNode(std::string(previous, c)), current));
                    previous = c;
                    current = current->parent;
                }
            } else if (expr == "elseif") {
                current = current->parent;
                previous = c;
                // auto ast = parse_parentesis(c, input.end());
                get_parenthesis(c, input.end());
                auto ast = nullptr;
                current->children.push_back(std::make_shared<prism::Node>(prism::ElseIfNode(ast), current));
                current = current->children.back();
                previous = c;
                bool sameLine = false;
                while(*c != '\n'){
                    if (*c == ';') {
                        sameLine = true;
                        break;
                    }
                    c++;
                }
                if(sameLine){
                    while (*c != '\n') {
                        c++;
                    }
                    current->children.push_back(std::make_shared<prism::Node>(prism::TextNode(std::string(previous, c)), current));
                    previous = c;
                    current = current->parent;
                }
            } else if (expr == "for") {
                // auto ast = parse_parentesis(c, input.end());
                get_parenthesis(c, input.end());
                auto ast = nullptr;
                current->children.push_back(std::make_shared<prism::Node>(prism::ForNode(ast), current));
                current = current->children.back();
                previous = c;
                bool sameLine = false;
                while(*c != '\n'){
                    if (*c == ';') {
                        sameLine = true;
                        break;
                    }
                    c++;
                }
                if(sameLine){
                    while (*c != '\n') {
                        c++;
                    }
                    current->children.push_back(std::make_shared<prism::Node>(prism::TextNode(std::string(previous, c)), current));
                    previous = c;
                    current = current->parent;
                }
            } else if (expr == "end") {
                current = current->parent;
                previous = c;
            }
        }
        c++;
    }

    if (current != root) {
        throw prism::SyntaxError("Unterminated block");
    }
    return *root;
}

void print_node(const prism::Node& node, int depth = 0) {
    for (int i = 0; i < depth; i++) {
        std::cout << ">";
    }
    if (std::holds_alternative<prism::TextNode>(node.node)) {
        std::cout << std::get<prism::TextNode>(node.node).text << std::endl;
    } else if (std::holds_alternative<prism::IfNode>(node.node)) {
        std::cout << "If" << std::endl;
        print_ast_node(std::get<prism::IfNode>(node.node).condition, depth + 1);
        for (const auto& child : node.children) {
            print_node(*child, depth + 1);
        }
        for (int i = 0; i < depth; i++) {
            std::cout << ">";
        }
        std::cout << "End" << std::endl;
    } else if (std::holds_alternative<prism::ElseNode>(node.node)) {
        std::cout << "Else" << std::endl;
        for (const auto& child : node.children) {
            print_node(*child, depth + 1);
        }
        for (int i = 0; i < depth; i++) {
            std::cout << ">";
        }
        std::cout << "End" << std::endl;
    } else if (std::holds_alternative<prism::ElseIfNode>(node.node)) {
        std::cout << "ElseIf" << std::endl;
        for (const auto& child : node.children) {
            print_node(*child, depth + 1);
        }
        for (int i = 0; i < depth; i++) {
            std::cout << ">";
        }
        std::cout << "End" << std::endl;
    } else if (std::holds_alternative<prism::ForNode>(node.node)) {
        std::cout << "For" << std::endl;
        for (const auto& child : node.children) {
            print_node(*child, depth + 1);
        }
        for (int i = 0; i < depth; i++) {
            std::cout << ">";
        }
        std::cout << "End" << std::endl;
    } else if (std::holds_alternative<prism::VariableNode>(node.node)) {
        std::cout << "Variable" << std::endl;
        print_ast_node(std::get<prism::VariableNode>(node.node).name, depth + 1);
    }
}

std::string prism::Processor::process() {
    auto node = parse(m_input);
    SPDLOG_INFO("Parsed node");
    for (const auto& child : node.children) {
        print_node(*child);
    }
    SPDLOG_INFO("Printed node");
    return m_output.str();
}