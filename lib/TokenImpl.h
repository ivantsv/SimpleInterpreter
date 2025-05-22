#pragma once

#include <regex>
#include <string>

enum class TokenType {
    kEMPTY,
    kIDENTIFIER,
    kNUMBER_INT,
    kNUMBER_FRAC,
    kNUMBER_EXP,
    kNUMBER_EXP_SIGN,
    kNUMBER_EXP_DIGITS,
    kOPERATOR,
    kSTRING,
    kSTRING_ESCAPE,
    kEND_STRING,
    kSPEC_SYMBOL,
    kCOMMENT,
    kKEYWORD
};

struct TokenPos {
    size_t row;
    size_t column;
};

inline std::ostream& operator<<(std::ostream& os, TokenPos pos) {
    os << pos.row + 1 << " " << pos.column + 1;
    return os;
}

struct Token {
    TokenType type;
    std::string text;
    TokenPos place;
};