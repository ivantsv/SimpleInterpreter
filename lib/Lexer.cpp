#include "Lexer.h"

#include <cctype>
#include <regex>
#include <unordered_set>
#include <iostream>

static const std::string operators = "*/%^=<>!";

static const std::unordered_set<std::string> operators_tokens = {
    "+", "-", "*", "/", "%",
    "^", "==", "!=", "<", ">",
    "<=", ">=", "=", "+=", "-=",
    "*=", "/=", "%=", "^=", "and",
    "or", "not"
};

static const std::unordered_set<std::string> key_words = {
    "if", "then", "else", "end", "while",
    "for", "in", "function", "return", "nil",
    "true", "false", "break", "continue"
};

static const std::unordered_set<std::string> spec_symbols = {
    "(", ")", "[", "]", ",", "//"
};

static bool is_char(char c) noexcept {
    return std::isalpha(c) || c == '_';
}

enum class Condition {
    kChar,
    kDigit,
    kDot,
    kEpsilon,
    kSign,
    kqMark,
    kSlash,
    kArithmetic,
    kComment,
    kFracStart,
    kNextRow,
    kAutoToken,
    kUnknown
};

static Condition get_condition(char c, char c2) noexcept {
    if (std::isdigit(c)) {
        return Condition::kDigit;
    } else if ((c == 'e' || c == 'E') && (c2 == '+' || c2 == '-' || isdigit(c2))) {
        return Condition::kEpsilon;
    } else if (is_char(c)) {
        return Condition::kChar;
    } else if (c == '+' || c == '-') {
        return Condition::kSign;
    } else if (c == '"') {
        return Condition::kqMark;
    } else if (c == '\\') {
        return Condition::kSlash;
    } else if (c == '/' && c2 == '/') {
        return Condition::kComment;
    } else if (operators.find(c) != std::string::npos) {
        return Condition::kArithmetic;
    } else if (c == '.' && std::isdigit(c2)) {
        return Condition::kFracStart;
    } else if (c == '.') {
        return Condition::kDot;
    } else if (c == '\n') {
        return Condition::kNextRow;
    } else if (c == '(' || c == ')' || c == ',' || c == '[' || c == ']') {
        return Condition::kAutoToken;
    }

    return Condition::kUnknown;
}

static State transition_function(State cur_state, Condition cur_condition) noexcept {
    if (cur_condition == Condition::kChar) {
        if (cur_state == State::kEMPTY || cur_state == State::kIDENTIFIER) return State::kIDENTIFIER;
        if (cur_state == State::kSTRING) return State::kSTRING;

    } else if (cur_condition == Condition::kDigit) {
        if (cur_state == State::kEMPTY || cur_state == State::kNUMBER_INT) return State::kNUMBER_INT;
        if (cur_state == State::kNUMBER_FRAC) return State::kNUMBER_FRAC;
        if (cur_state == State::kNUMBER_EXP || cur_state == State::kNUMBER_EXP_SIGN || cur_state == State::kNUMBER_EXP_DIGITS) return State::kNUMBER_EXP_DIGITS;

    } else if (cur_condition == Condition::kDot) {
        if (cur_state == State::kNUMBER_INT) return State::kNUMBER_FRAC;

    } else if (cur_condition == Condition::kEpsilon) {
        if (cur_state == State::kNUMBER_INT || cur_state == State::kNUMBER_FRAC) return State::kNUMBER_EXP;

    } else if (cur_condition == Condition::kSign) {
        if (cur_state == State::kNUMBER_EXP) return State::kNUMBER_EXP_SIGN;
        if (cur_state == State::kEMPTY) return State::kOPERATOR;

    } else if (cur_condition == Condition::kqMark) {
        if (cur_state == State::kEMPTY) return State::kSTRING;
        if (cur_state == State::kSTRING) return State::kEND_STRING;

    } else if (cur_condition == Condition::kSlash) {
        if (cur_state == State::kSTRING) return State::kSTRING_ESCAPE;

    } else if (cur_condition == Condition::kArithmetic) {
        if (cur_state == State::kEMPTY || cur_state == State::kOPERATOR) return State::kOPERATOR;

    } else if (cur_condition == Condition::kFracStart) {
        if (cur_state == State::kEMPTY || cur_state == State::kNUMBER_INT) return State::kNUMBER_FRAC;

    } else if (cur_condition == Condition::kComment) {
        if (cur_state == State::kEMPTY) return State::kCOMMENT;

    } else if (cur_condition == Condition::kNextRow) {
        if (cur_state == State::kCOMMENT) return State::kEMPTY;

    } else if (cur_condition == Condition::kAutoToken) {
        if (cur_state == State::kEMPTY) return State::kSPEC_SYMBOL;
    }

    if (cur_state == State::kSTRING || cur_state == State::kSTRING_ESCAPE) return State::kSTRING;

    return State::kEMPTY;
}

void LexerContext::Clear() noexcept {
    token = "";
    current_state_ = State::kEMPTY;
    column = 0;
    row = 0;
    index = 0;
}


void Lexer::LoadCode(const std::string &code) {
    code_ = code;
}

static void processing_redundant_symbol(char symbol, size_t& row, size_t& column) noexcept {
    if (symbol == '\n') {
        ++row;
        column = 0;
    } else {
        ++column;
    }
}

void Lexer::AddToken() {
    size_t token_column = context_.column - context_.token.size();
    TokenPos token_pos = {context_.row, (context_.current_state_ == State::kSTRING ? token_column - 2 : token_column)};
    Token token_to_add = {context_.current_state_, context_.token, token_pos};
    parsing_result_.push_back(token_to_add);
    context_.token = "";
    context_.current_state_ = State::kEMPTY;
}

void Lexer::EmptyStateProcessing() {
    if (key_words.contains(context_.token)) {
        context_.current_state_ = State::kKEYWORD;
    }

    if (context_.current_state_ == State::kEMPTY) {
        processing_redundant_symbol(code_[context_.index], context_.row, context_.column);
        ++context_.index;
        return;
    }

    if (context_.current_state_ == State::kEND_STRING) {
        context_.current_state_ = State::kSTRING;
        context_.token = context_.token.substr(1, context_.token.size() - 2);
    }

    AddToken();
}

void Lexer::CommentStateProcessing() noexcept {
    context_.current_state_ = State::kEMPTY;
    while (context_.index < code_.size()) {
        if (code_[context_.index] == '\n') {
            break;
        };
        ++context_.index;
    }
    ++context_.row;
    context_.column = 0;
    context_.token = "";
}

State Lexer::Transition() const noexcept {
    char symbol = (context_.index == code_.size() ? '\n' : code_[context_.index]);
    char next_symbol = (context_.index + 1 < code_.size() ? code_[context_.index + 1] : '\t');

    auto state = transition_function(context_.current_state_, get_condition(symbol, next_symbol));

    return state;
}

void Lexer::StateProcessing(State new_state) {
    if (new_state == State::kEMPTY) {
        EmptyStateProcessing();
        return;
    }

    context_.current_state_ = new_state;
    if (context_.current_state_ != State::kSTRING_ESCAPE) {
        context_.token += code_[context_.index];
        ++context_.column;
    }

    if (context_.current_state_ == State::kCOMMENT) {
        CommentStateProcessing();
    }

    ++context_.index;
}

void Lexer::Parse() {
    while (context_.index <= code_.size()) {

        auto state = Transition();
        StateProcessing(state);
    }

    context_.Clear();
}

static std::string token_type_to_str(TokenType token_type) noexcept {
    switch (token_type) {
        case TokenType::kEMPTY:
            return "kEMPTY";
        case TokenType::kIDENTIFIER:
            return "kIDENTIFIER";
        case TokenType::kNUMBER_INT:
            return "kNUMBER_INT";
        case TokenType::kNUMBER_FRAC:
            return "kNUMBER_FRAC";
        case TokenType::kNUMBER_EXP:
            return "kNUMBER_EXP";
        case TokenType::kNUMBER_EXP_SIGN:
            return "kNUMBER_EXP_SIGN";
        case TokenType::kNUMBER_EXP_DIGITS:
            return "kNUMBER_EXP_DIGITS";
        case TokenType::kOPERATOR:
            return "kOPERATOR";
        case TokenType::kSTRING:
            return "kSTRING";
        case TokenType::kSTRING_ESCAPE:
            return "kSTRING_ESCAPE";
        case TokenType::kSPEC_SYMBOL:
            return "kSPEC_SYMBOL";
        case TokenType::kCOMMENT:
            return "kCOMMENT";
        case TokenType::kKEYWORD:
            return "kKEYWORD";
        default:
            return "UNKNOWN_TOKEN_TYPE";
    }
}

void Lexer::PrintAllTokens() const noexcept {
    for (const auto& token : parsing_result_) {
        std::cout << token.text << " " << token_type_to_str(token.type) << " " << token.place << '\n';
    }
}

auto Lexer::GetParsingResult() const -> std::vector<Token> {
    return parsing_result_;
}