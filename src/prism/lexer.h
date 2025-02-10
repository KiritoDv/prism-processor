#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace prism::lexer {
    enum class TokenType {
        IDENTIFIER,  // Variable names
        INTEGER,     // Number
        FLOAT,       // Float
        LPAREN,      // '('
        RPAREN,      // ')'
        LBRACKET,    // '['
        RBRACKET,    // ']'
        COMMA,       // ','
        OR,          // '||'
        AND,         // '&&'
        EQUAL,       // '=='
        NOT,         // '!'
        IN,          // in
        IF,          // if
        THEN,        // then
        ELSE,        // else
        ELSEIF,      // elseif
        RANGE,       // ..
        END_OF_INPUT // End of script
    };

    struct Token {
        TokenType type;
        std::string value;

        Token(TokenType type, std::string value = "") : type(type), value(std::move(value)) {}
    };

    class Lexer {
    private:
        std::string input;
        size_t pos = 0;
    public:
        explicit Lexer(std::string text) : input(text) {}
        std::vector<Token> tokenize();
        size_t length() {
            return pos;
        }
    private:
        [[nodiscard]] char peek() const;
        std::string parseIdentifier();
        std::string parseNumber(bool* hasDecimalPoint);
    };
}