#pragma once

#include <vector>

#include "TokenImpl.h"

using State = TokenType;

struct LexerContext {
    std::string token;
    State current_state_ = State::kEMPTY;
    size_t column = 0;
    size_t row = 0;
    size_t index = 0;

    void Clear() noexcept;
};

class Lexer {
public:
    void Parse();

    void PrintAllTokens() const noexcept;

    auto GetParsingResult() const -> std::vector<Token>;

    void LoadCode(const std::string& code);

private:
    std::vector<Token> parsing_result_;
    std::string code_;

    LexerContext context_;

    void EmptyStateProcessing();
    void CommentStateProcessing() noexcept;

    State Transition() const noexcept;
    void StateProcessing(State new_state);
    void AddToken();
};