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

std::string prism::Processor::process() {
    for (const auto& line : m_lines) {
        if (line.empty()) {
            m_output << "\n";
            continue;
        }

        try {
            const auto expr = ev_expr(line);
            switch (expr) {
                case ExpressionType::If:
                    evaluate_if(line);
                    continue;
                case ExpressionType::For:
                    evaluate_for(line);
                    continue;
                case ExpressionType::End:
                    m_context.scope = ScopeType::None;
                    continue;
            }

            if(m_context.scope != ScopeType::None && !m_context.skipUntilEnd){
                m_output << line << "\n";
            }
        } catch (SyntaxError& e) {
            SPDLOG_ERROR("------------------ [ Prism Syntax Error ] ------------------");
            SPDLOG_ERROR("Error: {}", e.what());
            SPDLOG_ERROR("Where: {}", line);
            SPDLOG_ERROR("------------------------------------------------------------");
            break;
        } catch (RuntimeError& e) {
            spdlog::error("Runtime error: {}", e.what());
            break;
        }
    }

    return m_output.str();
}