#include "processor.h"

#include <spdlog/spdlog.h>
#include <sstream>
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
    for(const auto& n_line : m_lines){
        m_input += n_line + "\n";
    }
    SPDLOG_INFO("Prism script: {} v{} by {}", name, version, author);
}

template <typename T>
prism::ContextTypes ReadArrayByType(prism::Processor* proc, prism::ast::ArrayAccessNode& array) {
    auto var = proc->getTypes().at(array.name->name);
    if(!is_type(var, prism::MTDArray<T>)) {
        throw prism::SyntaxError(array.name->name + " is not an array");
    }
    prism::MTDArray<T> arrayVar = std::get<prism::MTDArray<T>>(var);
    auto indices = array.arrayIndices;
#define g_idx(x) std::get<int>(proc->evaluate(indices->at(x)))
    switch (indices->size()) {
        case 1: {
            return arrayVar.at(g_idx(0));
        }
        case 2:
            return arrayVar.at(g_idx(1), g_idx(0));
        case 3:
            return arrayVar.at(g_idx(2), g_idx(1), g_idx(0));
        case 4:
            return arrayVar.at(g_idx(3), g_idx(2), g_idx(1), g_idx(0));
    }
#undef g_idx
    throw prism::SyntaxError("We dont support array indexes bigger than 4");
};

prism::ContextTypes prism::Processor::evaluate(const std::shared_ptr<prism::ast::ASTNode>& node) {
    if (!node) return false;
    if (is_type(node->node, prism::ast::VariableNode)) {
        auto var = std::get<prism::ast::VariableNode>(node->node);
        if(!this->m_items.contains(var.name)){
            throw SyntaxError("Unknown variable " + var.name);
        }
        auto value = this->m_items.at(var.name);
        if(is_type(value, bool)){
            return std::get<bool>(value);
        }
        if(is_type(value, int)){
            return std::get<int>(value);
        }
        if(is_type(value, float)){
            return std::get<float>(value);
        }
        if(is_type(value, MTDArray<bool>)){
            return std::get<MTDArray<bool>>(value);
        }
        if(is_type(value, MTDArray<int>)){
            return std::get<MTDArray<int>>(value);
        }
        if(is_type(value, MTDArray<float>)){
            return std::get<MTDArray<float>>(value);
        }
        throw SyntaxError("Unsupported type");
    } else if (is_type(node->node, prism::ast::IntegerNode)) {
        return std::get<prism::ast::IntegerNode>(node->node).value;
    } else if (is_type(node->node, prism::ast::FloatNode)) {
        return std::get<prism::ast::FloatNode>(node->node).value;
    } else if (is_type(node->node, prism::ast::ArrayAccessNode)) {
        auto array = std::get<prism::ast::ArrayAccessNode>(node->node);
        if(!this->m_items.contains(array.name->name)){
            throw SyntaxError("Unknown variable " + array.name->name);
        }
        auto var = this->m_items.at(array.name->name);
        if(is_type(var, MTDArray<bool>)){
            return ReadArrayByType<bool>(this, array);
        }
        if(is_type(var, MTDArray<int>)){
            return ReadArrayByType<int>(this, array);
        }
        if(is_type(var, MTDArray<float>)){
            return ReadArrayByType<float>(this, array);
        }
        
        throw SyntaxError("Invalid array access");
    } else if (is_type(node->node, prism::ast::InNode)) {
        auto inNode = std::get<prism::ast::InNode>(node->node);
        auto left = evaluate(inNode.left);
        auto right = evaluate(inNode.right);
        if(is_type(left, prism::MTDArray<int>) && is_type(right, int)){
            auto leftArray = std::get<prism::MTDArray<int>>(left);
            auto rightValue = std::get<int>(right);
            for(auto i = 0; i < leftArray.dimensions[0]; i++){
                if(leftArray.at(i) == rightValue){
                    return true;
                }
            }
            return false;
        }
        if(is_type(left, prism::MTDArray<bool>) && is_type(right, bool)){
            auto leftArray = std::get<prism::MTDArray<bool>>(left);
            auto rightValue = std::get<bool>(right);
            for(auto i = 0; i < leftArray.dimensions[0]; i++){
                if(leftArray.at(i) == rightValue){
                    return true;
                }
            }
            return false;
        }
        if(is_type(left, prism::MTDArray<float>) && is_type(right, float)){
            auto leftArray = std::get<prism::MTDArray<float>>(left);
            auto rightValue = std::get<float>(right);
            for(auto i = 0; i < leftArray.dimensions[0]; i++){
                if(leftArray.at(i) == rightValue){
                    return true;
                }
            }
            return false;
        }
        throw SyntaxError("Invalid IN operation");
    } else if (is_type(node->node, prism::ast::OrNode)) {
        auto orNode = std::get<prism::ast::OrNode>(node->node);
        auto left = evaluate(orNode.left);
        auto right = evaluate(orNode.right);
        bool resLeft = false;
        bool resRight = false;
        if(is_type(left, bool)) {
            resLeft = std::get<bool>(left);
        }
        if(is_type(right, bool)) {
            resRight = std::get<bool>(right);
        }
        if(is_type(left, int)) {
            resLeft = std::get<int>(left) == 1;
        }
        if(is_type(right, int)) {
            resRight = std::get<int>(right) == 1;
        }
        if(is_type(left, float) || is_type(right, float)) {
            throw SyntaxError("Invalid AND operation, float are not supported");
        }
        return resLeft && resRight;
    } else if (is_type(node->node, prism::ast::AndNode)) {
        auto andNode = std::get<prism::ast::AndNode>(node->node);
        auto left = evaluate(andNode.left);
        auto right = evaluate(andNode.right);
        bool resLeft = false;
        bool resRight = false;
        if(is_type(left, bool)) {
            resLeft = std::get<bool>(left);
        }
        if(is_type(right, bool)) {
            resRight = std::get<bool>(right);
        }
        if(is_type(left, int)) {
            resLeft = std::get<int>(left) == 1;
        }
        if(is_type(right, int)) {
            resRight = std::get<int>(right) == 1;
        }
        if(is_type(left, float) || is_type(right, float)) {
            throw SyntaxError("Invalid AND operation, float are not supported");
        }
        return resLeft && resRight;
    } else if (is_type(node->node, prism::ast::EqualNode)) {
        auto equalNode = std::get<prism::ast::AndNode>(node->node);
        auto left = evaluate(equalNode.left);
        auto right = evaluate(equalNode.right);
        if(is_type(left, float) || is_type(right, float)) {
            throw SyntaxError("Invalid EQUAL operation, float are not supported");
        }

        if(is_type(left, bool) && is_type(right, bool)) {
            return std::get<bool>(left) == std::get<bool>(right);
        }

        if(is_type(left, int) && is_type(right, int)) {
            return std::get<int>(left) == std::get<int>(right);
        }

        if(is_type(left, int) && is_type(right, bool)) {
            return std::get<int>(left) == (std::get<bool>(right) ? 1 : 0);
        }

        if(is_type(left, bool) && is_type(right, int)) {
            return (std::get<bool>(left) ? 1 : 0) == std::get<int>(right);
        }

        throw SyntaxError("Invalid EQUAL operation");
    } else if (is_type(node->node, prism::ast::RangeNode)) {
        auto rangeNode = std::get<prism::ast::RangeNode>(node->node);
        auto start = evaluate(rangeNode.left);
        auto end = evaluate(rangeNode.right);
        if(!is_type(start, int) || !is_type(end, int)){
            throw SyntaxError("Invalid range");
        }

        auto entries = std::vector<int>();
        for(int i = std::get<int>(start); i < std::get<int>(end); i++){
            entries.push_back(i);
        }

        return prism::MTDArray<int>{(uintptr_t) entries.data(), std::vector<size_t>{ entries.size() }};
    } else if (is_type(node->node, prism::ast::IfNode)) {
        auto ifNode = std::get<prism::ast::IfNode>(node->node);
        auto condition = evaluate(ifNode.condition);

        if(is_type(condition, bool)) {
            return std::get<bool>(condition);
        }

        if(is_type(condition, int)) {
            return std::get<int>(condition) == 1;
        }

        throw SyntaxError("Invalid IF condition");
    } else if (is_type(node->node, prism::ast::ElseIfNode)) {
        auto ifNode = std::get<prism::ast::ElseIfNode>(node->node);
        auto condition = evaluate(ifNode.condition);

        if(is_type(condition, bool)) {
            return std::get<bool>(condition);
        }

        if(is_type(condition, int)) {
            return std::get<int>(condition) == 1;
        }

        throw SyntaxError("Invalid IF condition");
    } 
    return false;
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
    return {start, c};
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
    return {start, c};
}

prism::Node parse(std::string input) {
    std::shared_ptr<prism::Node> root = std::make_shared<prism::Node>(prism::TextNode{""}, nullptr);
    std::shared_ptr<prism::Node> current = root;
    auto c = input.begin();
    auto previous = c;
    while (c != input.end()) {
        if (*c == '@') {
            current->children.push_back(std::make_shared<prism::Node>(prism::TextNode{std::string(previous, c)}, current));
            c++;
            if (*c == '{') {
                auto ast = parse_accolade(c, input.end());
                // get_accolade(c, input.end());
                // auto ast = nullptr;
                current->children.push_back(std::make_shared<prism::Node>(prism::VariableNode{ast}, current));
            }
            auto expr = get_keyword(c, input.end());
            previous = c;
            if (expr == "if") {
                auto ast = parse_parentesis(c, input.end());
                // get_parenthesis(c, input.end());
                // auto ast = nullptr;
                current->children.push_back(std::make_shared<prism::Node>(prism::IfNode{ast}, current));

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
                    current->children.push_back(std::make_shared<prism::Node>(prism::TextNode{std::string(previous, c)}, current));
                    previous = c;
                    current = current->parent;
                }
            } else if (expr == "else") {
                current = current->parent;
                previous = c;
                current->children.push_back(std::make_shared<prism::Node>(prism::ElseNode{}, current));
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
                    current->children.push_back(std::make_shared<prism::Node>(prism::TextNode{std::string(previous, c)}, current));
                    previous = c;
                    current = current->parent;
                }
            } else if (expr == "elseif") {
                current = current->parent;
                previous = c;
                auto ast = parse_parentesis(c, input.end());
                // get_parenthesis(c, input.end());
                // auto ast = nullptr;
                current->children.push_back(std::make_shared<prism::Node>(prism::ElseIfNode{ast}, current));
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
                    current->children.push_back(std::make_shared<prism::Node>(prism::TextNode{std::string(previous, c)}, current));
                    previous = c;
                    current = current->parent;
                }
            } else if (expr == "for") {
                auto ast = parse_parentesis(c, input.end());
                // get_parenthesis(c, input.end());
                // auto ast = nullptr;
                current->children.push_back(std::make_shared<prism::Node>(prism::ForNode{ast}, current));
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
                    current->children.push_back(std::make_shared<prism::Node>(prism::TextNode{std::string(previous, c)}, current));
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
    if (is_type(node.node, prism::TextNode)) {
        std::cout << std::get<prism::TextNode>(node.node).text << std::endl;
    } else if (is_type(node.node, prism::IfNode)) {
        std::cout << "If" << std::endl;
        print_ast_node(std::get<prism::IfNode>(node.node).condition, depth + 1);
        for (const auto& child : node.children) {
            print_node(*child, depth + 1);
        }
        for (int i = 0; i < depth; i++) {
            std::cout << ">";
        }
        std::cout << "End" << std::endl;
    } else if (is_type(node.node, prism::ElseNode)) {
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
        print_ast_node(std::get<prism::ElseIfNode>(node.node).condition, depth + 1);
        for (const auto& child : node.children) {
            print_node(*child, depth + 1);
        }
        for (int i = 0; i < depth; i++) {
            std::cout << ">";
        }
        std::cout << "End" << std::endl;
    } else if (std::holds_alternative<prism::ForNode>(node.node)) {
        std::cout << "For" << std::endl;
        print_ast_node(std::get<prism::ForNode>(node.node).condition, depth + 1);
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