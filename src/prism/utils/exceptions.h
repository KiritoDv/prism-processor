#pragma once

#include <exception>

namespace prism {
    class SyntaxError : public std::exception {
    public:
        SyntaxError(const char* message) : m_message(message) {}
        const char* what() const noexcept override {
            return m_message;
        }
    private:
        const char* m_message;
    };

    class RuntimeError : public std::exception {
    public:
        RuntimeError(const char* message) : m_message(message) {}
        const char* what() const noexcept override {
            return m_message;
        }
    private:
        const char* m_message;
    };
}