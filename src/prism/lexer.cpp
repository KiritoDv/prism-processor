#include "lexer.h"
#include "utils/exceptions.h"

std::vector<prism::lexer::Token> prism::lexer::Lexer::tokenize() {
    std::vector<prism::lexer::Token> tokens;

    while (pos < input.size()) {
        char current = input[pos];

        if (std::isspace(current)) {
            pos++;  // Skip whitespace
            continue;
        }

        if (input.substr(pos, 2) != "in" && std::isalpha(current) || current == '_') {
            tokens.emplace_back(TokenType::IDENTIFIER, parseIdentifier());
            continue;
        }

        if (std::isdigit(current)) {
            bool hasDecimal = false;
            auto result = parseNumber(&hasDecimal);

            tokens.emplace_back(hasDecimal ? TokenType::FLOAT : TokenType::INTEGER, result);
            continue;
        }

        switch (current) {
            case '(': tokens.emplace_back(TokenType::LPAREN, "("); pos++; break;
            case ')': tokens.emplace_back(TokenType::RPAREN, ")"); pos++; break;
            case ',': tokens.emplace_back(TokenType::COMMA, ","); pos++; break;
            case '[': tokens.emplace_back(TokenType::LBRACKET, "["); pos++; break;
            case ']': tokens.emplace_back(TokenType::RBRACKET, "]"); pos++; break;
            case '!': tokens.emplace_back(TokenType::NOT, "!"); pos++; break;
            case '|':
                if (peek() == '|') { tokens.emplace_back(TokenType::OR, "||"); pos += 2; }
                break;
            case '&':
                if (peek() == '&') { tokens.emplace_back(TokenType::AND, "&&"); pos += 2; }
                break;
            case '=':
                if (peek() == '=') { tokens.emplace_back(TokenType::EQUAL, "=="); pos += 2; }
                break;
            case 'i':
                if (peek() == 'n') { tokens.emplace_back(TokenType::IN, "in"); pos += 2; }
                else if (peek() == 'f') { tokens.emplace_back(TokenType::IF, "if"); pos += 2; }
                break;
            case 'e':
                if (peek() == 'l') {
                    if (input.substr(pos, 4) == "else") {
                        tokens.emplace_back(TokenType::ELSE, "else");
                        pos += 4;
                    } else if (input.substr(pos, 6) == "elseif") {
                        tokens.emplace_back(TokenType::ELSEIF, "elseif");
                        pos += 6;
                    }
                }
                break;
            case 't':
                if (peek() == 'h') { tokens.emplace_back(TokenType::THEN, "then"); pos += 4; }
                break;
            case '.':
                if (peek() == '.') { tokens.emplace_back(TokenType::RANGE, ".."); pos += 2; }
                if (std::isalpha(peek())) { tokens.emplace_back(TokenType::IDENTIFIER, parseIdentifier()); }
                break;
            case ';': pos++; break;
            default:
                throw prism::SyntaxError("Unexpected character " + std::string(1, current));
        }
    }

    tokens.emplace_back(TokenType::END_OF_INPUT);
    return tokens;
}

char prism::lexer::Lexer::peek() const {
    return (pos + 1 < input.size()) ? input[pos + 1] : '\0';
}

std::string prism::lexer::Lexer::parseIdentifier() {
    size_t start = pos;
    while (pos < input.size() && (std::isalnum(input[pos]) || input[pos] == '_')) {
        pos++;
    }
    return input.substr(start, pos - start);
}

std::string prism::lexer::Lexer::parseNumber(bool* hasDecimalPoint) {
    size_t start = pos;

    while (pos < input.size()) {
        char currentChar = input[pos];

        // If it's a digit, we continue parsing
        if (std::isdigit(currentChar)) {
            pos++;
        } else if (currentChar == '.' && !hasDecimalPoint) {
            (*hasDecimalPoint) = true;
            pos++;
        } else {
            break;
        }
    }

    return input.substr(start, pos - start);
}
