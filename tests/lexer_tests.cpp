#include <gtest/gtest.h>
#include "Lexer.h"
#include <sstream>
#include <fstream>

class LexerTests : public ::testing::Test {
public:
    // поля общие для всех тестов
    Lexer lexer;
    std::vector<Token> tokens;

    // штука, которая запускается перед каждым тестом
    void SetUp() override {
        tokens.clear();
    }

    void runLexer(const std::string& code) {
        lexer = Lexer();
        lexer.LoadCode(code);
        lexer.Parse();
        tokens = lexer.GetParsingResult();
    }

    void expectToken(size_t index, TokenType type, const std::string& text, size_t row, size_t column) {
        ASSERT_LT(index, tokens.size()) << "Мало токенов, ожидали " << text;
        EXPECT_EQ(tokens[index].type, type) << "Неправильный тип для токена " << text;
        EXPECT_EQ(tokens[index].text, text) << "Неправильный текст для токена на индексе " << index;
        EXPECT_EQ(tokens[index].place.row, row) << "Неправильная строка для токена " << text;
        EXPECT_EQ(tokens[index].place.column, column) << "Неправильный столбец для токена " << text;
    }
};

// TEST_F - тест с фикстурой (LexerTests)
TEST_F(LexerTests, SimpleIdentifier) {
    runLexer("hello");
    ASSERT_EQ(tokens.size(), 1);
    expectToken(0, TokenType::kIDENTIFIER, "hello", 0, 0);
}

TEST_F(LexerTests, Keyword) {
    runLexer("if");
    ASSERT_EQ(tokens.size(), 1);
    expectToken(0, TokenType::kKEYWORD, "if", 0, 0);
}

TEST_F(LexerTests, IntegerNumber) {
    runLexer("123");
    ASSERT_EQ(tokens.size(), 1);
    expectToken(0, TokenType::kNUMBER_INT, "123", 0, 0);
}

TEST_F(LexerTests, FractionalNumber) {
    runLexer("12.34");
    ASSERT_EQ(tokens.size(), 1);
    expectToken(0, TokenType::kNUMBER_FRAC, "12.34", 0, 0);
}

TEST_F(LexerTests, ExponentialNumber) {
    runLexer("1.23e-4");
    ASSERT_EQ(tokens.size(), 1);
    expectToken(0, TokenType::kNUMBER_EXP_DIGITS, "1.23e-4", 0, 0);
}

TEST_F(LexerTests, SimpleString) {
    runLexer("\"hello\"");
    ASSERT_EQ(tokens.size(), 1);
    expectToken(0, TokenType::kSTRING, "hello", 0, 0);
}

TEST_F(LexerTests, StringWithEscape) {
    runLexer("\"hello \\\"world\\\"\"");
    ASSERT_EQ(tokens.size(), 1);
    expectToken(0, TokenType::kSTRING, "hello \"world\"", 0, 0);
}

TEST_F(LexerTests, Operators) {
    runLexer("+= -= == != <= >=");
    ASSERT_EQ(tokens.size(), 6);
    expectToken(0, TokenType::kOPERATOR, "+=", 0, 0);
    expectToken(1, TokenType::kOPERATOR, "-=", 0, 3);
    expectToken(2, TokenType::kOPERATOR, "==", 0, 6);
    expectToken(3, TokenType::kOPERATOR, "!=", 0, 9);
    expectToken(4, TokenType::kOPERATOR, "<=", 0, 12);
    expectToken(5, TokenType::kOPERATOR, ">=", 0, 15);
}

TEST_F(LexerTests, SpecialSymbols) {
    runLexer("( ) , [ ]");
    ASSERT_EQ(tokens.size(), 5);
    expectToken(0, TokenType::kSPEC_SYMBOL, "(", 0, 0);
    expectToken(1, TokenType::kSPEC_SYMBOL, ")", 0, 2);
    expectToken(2, TokenType::kSPEC_SYMBOL, ",", 0, 4);
    expectToken(3, TokenType::kSPEC_SYMBOL, "[", 0, 6);
    expectToken(4, TokenType::kSPEC_SYMBOL, "]", 0, 8);
}

TEST_F(LexerTests, Comment) {
    runLexer("x = 1 // comment\ny = 2");
    ASSERT_EQ(tokens.size(), 6);
    expectToken(0, TokenType::kIDENTIFIER, "x", 0, 0);
    expectToken(1, TokenType::kOPERATOR, "=", 0, 2);
    expectToken(2, TokenType::kNUMBER_INT, "1", 0, 4);
    expectToken(3, TokenType::kIDENTIFIER, "y", 1, 0);
    expectToken(4, TokenType::kOPERATOR, "=", 1, 2);
    expectToken(5, TokenType::kNUMBER_INT, "2", 1, 4);
}

TEST_F(LexerTests, WhitespaceAndNewlines) {
    runLexer("  x   \n  y  ");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(0, TokenType::kIDENTIFIER, "x", 0, 2);
    expectToken(1, TokenType::kIDENTIFIER, "y", 1, 2);
}

TEST_F(LexerTests, ComplexExpression) {
    runLexer("fib = function(n)\n    if n == 0 then");
    ASSERT_EQ(tokens.size(), 11);
    expectToken(0, TokenType::kIDENTIFIER, "fib", 0, 0);
    expectToken(1, TokenType::kOPERATOR, "=", 0, 4);
    expectToken(2, TokenType::kKEYWORD, "function", 0, 6);
    expectToken(3, TokenType::kSPEC_SYMBOL, "(", 0, 14);
    expectToken(4, TokenType::kIDENTIFIER, "n", 0, 15);
    expectToken(5, TokenType::kSPEC_SYMBOL, ")", 0, 16);
    expectToken(6, TokenType::kKEYWORD, "if", 1, 4);
    expectToken(7, TokenType::kIDENTIFIER, "n", 1, 7);
    expectToken(8, TokenType::kOPERATOR, "==", 1, 9);
    expectToken(9, TokenType::kNUMBER_INT, "0", 1, 12);
    expectToken(10, TokenType::kKEYWORD, "then", 1, 14);
}

TEST_F(LexerTests, Indexing) {
    runLexer("arr[10]");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(0, TokenType::kIDENTIFIER, "arr", 0, 0);
    expectToken(1, TokenType::kSPEC_SYMBOL, "[", 0, 3);
    expectToken(2, TokenType::kNUMBER_INT, "10", 0, 4);
    expectToken(3, TokenType::kSPEC_SYMBOL, "]", 0, 6);
}

TEST_F(LexerTests, EmptyInput) {
    runLexer("");
    ASSERT_EQ(tokens.size(), 0);
}

TEST_F(LexerTests, InvalidSymbol) {
    runLexer("x # y");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(0, TokenType::kIDENTIFIER, "x", 0, 0);
    expectToken(1, TokenType::kIDENTIFIER, "y", 0, 4);
}

class LexerFileTests : public ::testing::Test {
public:
    Lexer lexer;
    std::vector<Token> tokens;

    void SetUp() override {
        tokens.clear();
    }

    void runLexer(const std::string& code) {
        lexer = Lexer();
        lexer.LoadCode(code);
        lexer.Parse();
        tokens = lexer.GetParsingResult();
    }

    void expectToken(size_t index, TokenType type, const std::string& text, size_t row, size_t column) {
        ASSERT_LT(index, tokens.size()) << "Мало токенов, ожидали токен '" << text << "'";
        EXPECT_EQ(tokens[index].type, type) << "Неправильный тип для токена '" << text << "' на индексе " << index;
        EXPECT_EQ(tokens[index].text, text) << "Неправильный текст для токена на индексе " << index;
        EXPECT_EQ(tokens[index].place.row, row) << "Неправильная строка для токена '" << text << "'";
        EXPECT_EQ(tokens[index].place.column, column) << "Неправильный столбец для токена '" << text << "'";
    }
};

TEST_F(LexerFileTests, MaximumIs) {
    std::ifstream code_file("/home/tsvetkov-ivan/cpp_labs/labwork10-ivantsv/examples/maximum.is");

    std::stringstream code;
    code << code_file.rdbuf();

    runLexer(code.str());
    ASSERT_EQ(tokens.size(), 64);

    expectToken(0, TokenType::kIDENTIFIER, "max", 0, 0);
    expectToken(1, TokenType::kOPERATOR, "=", 0, 4);
    expectToken(2, TokenType::kKEYWORD, "function", 0, 6);
    expectToken(3, TokenType::kSPEC_SYMBOL, "(", 0, 14);
    expectToken(4, TokenType::kIDENTIFIER, "arr", 0, 15);
    expectToken(5, TokenType::kSPEC_SYMBOL, ")", 0, 18);
    expectToken(6, TokenType::kKEYWORD, "if", 1, 4);
    expectToken(7, TokenType::kIDENTIFIER, "len", 1, 7);
    expectToken(8, TokenType::kSPEC_SYMBOL, "(", 1, 10);
    expectToken(9, TokenType::kIDENTIFIER, "arr", 1, 11);
    expectToken(10, TokenType::kSPEC_SYMBOL, ")", 1, 14);
    expectToken(11, TokenType::kOPERATOR, "==", 1, 16);
    expectToken(12, TokenType::kNUMBER_INT, "0", 1, 19);
    expectToken(13, TokenType::kKEYWORD, "then", 1, 21);
    expectToken(14, TokenType::kKEYWORD, "return", 2, 8);
    expectToken(15, TokenType::kKEYWORD, "nil", 2, 15);
    expectToken(16, TokenType::kKEYWORD, "end", 3, 4);
    expectToken(17, TokenType::kKEYWORD, "if", 3, 8);
    expectToken(18, TokenType::kIDENTIFIER, "m", 5, 4);
    expectToken(19, TokenType::kOPERATOR, "=", 5, 6);
    expectToken(20, TokenType::kIDENTIFIER, "arr", 5, 8);
    expectToken(21, TokenType::kSPEC_SYMBOL, "[", 5, 11);
    expectToken(22, TokenType::kNUMBER_INT, "0", 5, 12);
    expectToken(23, TokenType::kSPEC_SYMBOL, "]", 5, 13);
    expectToken(24, TokenType::kKEYWORD, "for", 7, 4);
    expectToken(25, TokenType::kIDENTIFIER, "i", 7, 8);
    expectToken(26, TokenType::kKEYWORD, "in", 7, 10);
    expectToken(27, TokenType::kIDENTIFIER, "arr", 7, 13);
    expectToken(28, TokenType::kKEYWORD, "if", 8, 8);
    expectToken(29, TokenType::kIDENTIFIER, "i", 8, 11);
    expectToken(30, TokenType::kOPERATOR, ">", 8, 13);
    expectToken(31, TokenType::kIDENTIFIER, "m", 8, 15);
    expectToken(32, TokenType::kKEYWORD, "then", 8, 17);
    expectToken(33, TokenType::kIDENTIFIER, "m", 8, 22);
    expectToken(34, TokenType::kOPERATOR, "=", 8, 24);
    expectToken(35, TokenType::kIDENTIFIER, "i", 8, 26);
    expectToken(36, TokenType::kKEYWORD, "end", 8, 28);
    expectToken(37, TokenType::kKEYWORD, "if", 8, 32);
    expectToken(38, TokenType::kKEYWORD, "end", 9, 4);
    expectToken(39, TokenType::kKEYWORD, "for", 9, 8);
    expectToken(40, TokenType::kKEYWORD, "return", 11, 4);
    expectToken(41, TokenType::kIDENTIFIER, "m", 11, 11);
    expectToken(42, TokenType::kKEYWORD, "end", 12, 0);
    expectToken(43, TokenType::kKEYWORD, "function", 12, 4);
    expectToken(44, TokenType::kIDENTIFIER, "print", 14, 0);
    expectToken(45, TokenType::kSPEC_SYMBOL, "(", 14, 5);
    expectToken(46, TokenType::kIDENTIFIER, "max", 14, 6);
    expectToken(47, TokenType::kSPEC_SYMBOL, "(", 14, 9);
    expectToken(48, TokenType::kSPEC_SYMBOL, "[", 14, 10);
    expectToken(49, TokenType::kNUMBER_INT, "10", 14, 11);
    expectToken(50, TokenType::kSPEC_SYMBOL, ",", 14, 13);
    expectToken(51, TokenType::kOPERATOR, "-", 14, 15);
    expectToken(52, TokenType::kNUMBER_INT, "1", 14, 16);
    expectToken(53, TokenType::kSPEC_SYMBOL, ",", 14, 17);
    expectToken(54, TokenType::kNUMBER_INT, "0", 14, 19);
    expectToken(55, TokenType::kSPEC_SYMBOL, ",", 14, 20);
    expectToken(56, TokenType::kNUMBER_INT, "2", 14, 22);
    expectToken(57, TokenType::kSPEC_SYMBOL, ",", 14, 23);
    expectToken(58, TokenType::kNUMBER_INT, "2025", 14, 25);
    expectToken(59, TokenType::kSPEC_SYMBOL, ",", 14, 29);
    expectToken(60, TokenType::kNUMBER_INT, "239", 14, 31);
    expectToken(61, TokenType::kSPEC_SYMBOL, "]", 14, 34);
    expectToken(62, TokenType::kSPEC_SYMBOL, ")", 14, 35);
    expectToken(63, TokenType::kSPEC_SYMBOL, ")", 14, 36);
}

TEST_F(LexerFileTests, FizzBuzzIs) {
    std::ifstream code_file("/home/tsvetkov-ivan/cpp_labs/labwork10-ivantsv/examples/fizzBuzz.is");

    std::stringstream code;
    code << code_file.rdbuf();

    runLexer(code.str());
    ASSERT_EQ(tokens.size(), 60);

    expectToken(0, TokenType::kIDENTIFIER, "fizzBuzz", 0, 0);
    expectToken(1, TokenType::kOPERATOR, "=", 0, 9);
    expectToken(2, TokenType::kKEYWORD, "function", 0, 11);
    expectToken(3, TokenType::kSPEC_SYMBOL, "(", 0, 19);
    expectToken(4, TokenType::kIDENTIFIER, "n", 0, 20);
    expectToken(5, TokenType::kSPEC_SYMBOL, ")", 0, 21);
    expectToken(6, TokenType::kKEYWORD, "for", 1, 4);
    expectToken(7, TokenType::kIDENTIFIER, "i", 1, 8);
    expectToken(8, TokenType::kKEYWORD, "in", 1, 10);
    expectToken(9, TokenType::kIDENTIFIER, "range", 1, 13);
    expectToken(10, TokenType::kSPEC_SYMBOL, "(", 1, 18);
    expectToken(11, TokenType::kNUMBER_INT, "1", 1, 19);
    expectToken(12, TokenType::kSPEC_SYMBOL, ",", 1, 20);
    expectToken(13, TokenType::kIDENTIFIER, "n", 1, 22);
    expectToken(14, TokenType::kSPEC_SYMBOL, ")", 1, 23);
    expectToken(15, TokenType::kIDENTIFIER, "s", 2, 8);
    expectToken(16, TokenType::kOPERATOR, "=", 2, 10);
    expectToken(17, TokenType::kSTRING, "Fizz", 2, 12);
    expectToken(18, TokenType::kOPERATOR, "*", 2, 19);
    expectToken(19, TokenType::kSPEC_SYMBOL, "(", 2, 21);
    expectToken(20, TokenType::kIDENTIFIER, "i", 2, 22);
    expectToken(21, TokenType::kOPERATOR, "%", 2, 24);
    expectToken(22, TokenType::kNUMBER_INT, "3", 2, 26);
    expectToken(23, TokenType::kOPERATOR, "==", 2, 28);
    expectToken(24, TokenType::kNUMBER_INT, "0", 2, 31);
    expectToken(25, TokenType::kSPEC_SYMBOL, ")", 2, 32);
    expectToken(26, TokenType::kOPERATOR, "+", 2, 34);
    expectToken(27, TokenType::kSTRING, "Buzz", 2, 36);
    expectToken(28, TokenType::kOPERATOR, "*", 2, 43);
    expectToken(29, TokenType::kSPEC_SYMBOL, "(", 2, 45);
    expectToken(30, TokenType::kIDENTIFIER, "i", 2, 46);
    expectToken(31, TokenType::kOPERATOR, "%", 2, 48);
    expectToken(32, TokenType::kNUMBER_INT, "5", 2, 50);
    expectToken(33, TokenType::kOPERATOR, "==", 2, 52);
    expectToken(34, TokenType::kNUMBER_INT, "0", 2, 55);
    expectToken(35, TokenType::kSPEC_SYMBOL, ")", 2, 56);
    expectToken(36, TokenType::kKEYWORD, "if", 3, 8);
    expectToken(37, TokenType::kIDENTIFIER, "s", 3, 11);
    expectToken(38, TokenType::kOPERATOR, "==", 3, 13);
    expectToken(39, TokenType::kSTRING, "", 3, 16);
    expectToken(40, TokenType::kKEYWORD, "then", 3, 19);
    expectToken(41, TokenType::kIDENTIFIER, "print", 4, 12);
    expectToken(42, TokenType::kSPEC_SYMBOL, "(", 4, 17);
    expectToken(43, TokenType::kIDENTIFIER, "i", 4, 18);
    expectToken(44, TokenType::kSPEC_SYMBOL, ")", 4, 19);
    expectToken(45, TokenType::kKEYWORD, "else", 5, 8);
    expectToken(46, TokenType::kIDENTIFIER, "print", 6, 12);
    expectToken(47, TokenType::kSPEC_SYMBOL, "(", 6, 17);
    expectToken(48, TokenType::kIDENTIFIER, "s", 6, 18);
    expectToken(49, TokenType::kSPEC_SYMBOL, ")", 6, 19);
    expectToken(50, TokenType::kKEYWORD, "end", 7, 8);
    expectToken(51, TokenType::kKEYWORD, "if", 7, 12);
    expectToken(52, TokenType::kKEYWORD, "end", 8, 4);
    expectToken(53, TokenType::kKEYWORD, "for", 8, 8);
    expectToken(54, TokenType::kKEYWORD, "end", 9, 0);
    expectToken(55, TokenType::kKEYWORD, "function", 9, 4);
    expectToken(56, TokenType::kIDENTIFIER, "fizzBuzz", 11, 0);
    expectToken(57, TokenType::kSPEC_SYMBOL, "(", 11, 8);
    expectToken(58, TokenType::kNUMBER_INT, "100", 11, 9);
    expectToken(59, TokenType::kSPEC_SYMBOL, ")", 11, 12);
}

TEST_F(LexerFileTests, FibonacciIs) {
    std::ifstream code_file("/home/tsvetkov-ivan/cpp_labs/labwork10-ivantsv/examples/fibonacci.is");

    std::stringstream code;
    code << code_file.rdbuf();

    runLexer(code.str());
    ASSERT_EQ(tokens.size(), 54);

    expectToken(0, TokenType::kIDENTIFIER, "fib", 0, 0);
    expectToken(1, TokenType::kOPERATOR, "=", 0, 4);
    expectToken(2, TokenType::kKEYWORD, "function", 0, 6);
    expectToken(3, TokenType::kSPEC_SYMBOL, "(", 0, 14);
    expectToken(4, TokenType::kIDENTIFIER, "n", 0, 15);
    expectToken(5, TokenType::kSPEC_SYMBOL, ")", 0, 16);
    expectToken(6, TokenType::kKEYWORD, "if", 1, 4);
    expectToken(7, TokenType::kIDENTIFIER, "n", 1, 7);
    expectToken(8, TokenType::kOPERATOR, "==", 1, 9);
    expectToken(9, TokenType::kNUMBER_INT, "0", 1, 12);
    expectToken(10, TokenType::kKEYWORD, "then", 1, 14);
    expectToken(11, TokenType::kKEYWORD, "return", 2, 8);
    expectToken(12, TokenType::kNUMBER_INT, "0", 2, 15);
    expectToken(13, TokenType::kKEYWORD, "end", 3, 4);
    expectToken(14, TokenType::kKEYWORD, "if", 3, 8);
    expectToken(15, TokenType::kIDENTIFIER, "a", 5, 4);
    expectToken(16, TokenType::kOPERATOR, "=", 5, 6);
    expectToken(17, TokenType::kNUMBER_INT, "0", 5, 8);
    expectToken(18, TokenType::kIDENTIFIER, "b", 6, 4);
    expectToken(19, TokenType::kOPERATOR, "=", 6, 6);
    expectToken(20, TokenType::kNUMBER_INT, "1", 6, 8);
    expectToken(21, TokenType::kKEYWORD, "for", 8, 4);
    expectToken(22, TokenType::kIDENTIFIER, "i", 8, 8);
    expectToken(23, TokenType::kKEYWORD, "in", 8, 10);
    expectToken(24, TokenType::kIDENTIFIER, "range", 8, 13);
    expectToken(25, TokenType::kSPEC_SYMBOL, "(", 8, 18);
    expectToken(26, TokenType::kIDENTIFIER, "n", 8, 19);
    expectToken(27, TokenType::kOPERATOR, "-", 8, 21);
    expectToken(28, TokenType::kNUMBER_INT, "1", 8, 23);
    expectToken(29, TokenType::kSPEC_SYMBOL, ")", 8, 24);
    expectToken(30, TokenType::kIDENTIFIER, "c", 9, 8);
    expectToken(31, TokenType::kOPERATOR, "=", 9, 10);
    expectToken(32, TokenType::kIDENTIFIER, "a", 9, 12);
    expectToken(33, TokenType::kOPERATOR, "+", 9, 14);
    expectToken(34, TokenType::kIDENTIFIER, "b", 9, 16);
    expectToken(35, TokenType::kIDENTIFIER, "a", 10, 8);
    expectToken(36, TokenType::kOPERATOR, "=", 10, 10);
    expectToken(37, TokenType::kIDENTIFIER, "b", 10, 12);
    expectToken(38, TokenType::kIDENTIFIER, "b", 11, 8);
    expectToken(39, TokenType::kOPERATOR, "=", 11, 10);
    expectToken(40, TokenType::kIDENTIFIER, "c", 11, 12);
    expectToken(41, TokenType::kKEYWORD, "end", 12, 4);
    expectToken(42, TokenType::kKEYWORD, "for", 12, 8);
    expectToken(43, TokenType::kKEYWORD, "return", 14, 4);
    expectToken(44, TokenType::kIDENTIFIER, "b", 14, 11);
    expectToken(45, TokenType::kKEYWORD, "end", 15, 0);
    expectToken(46, TokenType::kKEYWORD, "function", 15, 4);
    expectToken(47, TokenType::kIDENTIFIER, "print", 18, 0);
    expectToken(48, TokenType::kSPEC_SYMBOL, "(", 18, 5);
    expectToken(49, TokenType::kIDENTIFIER, "fib", 18, 6);
    expectToken(50, TokenType::kSPEC_SYMBOL, "(", 18, 9);
    expectToken(51, TokenType::kNUMBER_INT, "10", 18, 10);
    expectToken(52, TokenType::kSPEC_SYMBOL, ")", 18, 12);
    expectToken(53, TokenType::kSPEC_SYMBOL, ")", 18, 13);
}

TEST_F(LexerFileTests, FibonacciWithTabsIs) {
    std::ifstream code_file("/home/tsvetkov-ivan/cpp_labs/labwork10-ivantsv/examples/fizzBuzz_with_tabs.is");

    std::stringstream code;
    code << code_file.rdbuf();

    runLexer(code.str());
    ASSERT_EQ(tokens.size(), 60);

    expectToken(0, TokenType::kIDENTIFIER, "fizzBuzz", 0, 0);
    expectToken(1, TokenType::kOPERATOR, "=", 0, 9);
    expectToken(2, TokenType::kKEYWORD, "function", 0, 11);
    expectToken(3, TokenType::kSPEC_SYMBOL, "(", 0, 19);
    expectToken(4, TokenType::kIDENTIFIER, "n", 0, 20);
    expectToken(5, TokenType::kSPEC_SYMBOL, ")", 0, 21);
    expectToken(6, TokenType::kKEYWORD, "for", 1, 44);
    expectToken(7, TokenType::kIDENTIFIER, "i", 1, 48);
    expectToken(8, TokenType::kKEYWORD, "in", 1, 50);
    expectToken(9, TokenType::kIDENTIFIER, "range", 1, 53);
    expectToken(10, TokenType::kSPEC_SYMBOL, "(", 1, 58);
    expectToken(11, TokenType::kNUMBER_INT, "1", 1, 59);
    expectToken(12, TokenType::kSPEC_SYMBOL, ",", 1, 60);
    expectToken(13, TokenType::kIDENTIFIER, "n", 1, 62);
    expectToken(14, TokenType::kSPEC_SYMBOL, ")", 1, 63);
    expectToken(15, TokenType::kIDENTIFIER, "s", 2, 8);
    expectToken(16, TokenType::kOPERATOR, "=", 2, 10);
    expectToken(17, TokenType::kSTRING, "Fizz", 2, 12);
    expectToken(18, TokenType::kOPERATOR, "*", 2, 19);
    expectToken(19, TokenType::kSPEC_SYMBOL, "(", 2, 21);
    expectToken(20, TokenType::kIDENTIFIER, "i", 2, 22);
    expectToken(21, TokenType::kOPERATOR, "%", 2, 24);
    expectToken(22, TokenType::kNUMBER_INT, "3", 2, 26);
    expectToken(23, TokenType::kOPERATOR, "==", 2, 28);
    expectToken(24, TokenType::kNUMBER_INT, "0", 2, 31);
    expectToken(25, TokenType::kSPEC_SYMBOL, ")", 2, 32);
    expectToken(26, TokenType::kOPERATOR, "+", 2, 34);
    expectToken(27, TokenType::kSTRING, "Buzz", 2, 36);
    expectToken(28, TokenType::kOPERATOR, "*", 2, 43);
    expectToken(29, TokenType::kSPEC_SYMBOL, "(", 2, 45);
    expectToken(30, TokenType::kIDENTIFIER, "i", 2, 46);
    expectToken(31, TokenType::kOPERATOR, "%", 2, 48);
    expectToken(32, TokenType::kNUMBER_INT, "5", 2, 50);
    expectToken(33, TokenType::kOPERATOR, "==", 2, 52);
    expectToken(34, TokenType::kNUMBER_INT, "0", 2, 55);
    expectToken(35, TokenType::kSPEC_SYMBOL, ")", 2, 56);
    expectToken(36, TokenType::kKEYWORD, "if", 3, 35);
    expectToken(37, TokenType::kIDENTIFIER, "s", 3, 38);
    expectToken(38, TokenType::kOPERATOR, "==", 3, 65);
    expectToken(39, TokenType::kSTRING, "", 3, 68);
    expectToken(40, TokenType::kKEYWORD, "then", 3, 71);
    expectToken(41, TokenType::kIDENTIFIER, "print", 4, 12);
    expectToken(42, TokenType::kSPEC_SYMBOL, "(", 4, 17);
    expectToken(43, TokenType::kIDENTIFIER, "i", 4, 18);
    expectToken(44, TokenType::kSPEC_SYMBOL, ")", 4, 19);
    expectToken(45, TokenType::kKEYWORD, "else", 5, 8);
    expectToken(46, TokenType::kIDENTIFIER, "print", 6, 24);
    expectToken(47, TokenType::kSPEC_SYMBOL, "(", 6, 29);
    expectToken(48, TokenType::kIDENTIFIER, "s", 6, 30);
    expectToken(49, TokenType::kSPEC_SYMBOL, ")", 6, 31);
    expectToken(50, TokenType::kKEYWORD, "end", 7, 8);
    expectToken(51, TokenType::kKEYWORD, "if", 7, 12);
    expectToken(52, TokenType::kKEYWORD, "end", 8, 4);
    expectToken(53, TokenType::kKEYWORD, "for", 8, 8);
    expectToken(54, TokenType::kKEYWORD, "end", 9, 0);
    expectToken(55, TokenType::kKEYWORD, "function", 9, 4);
    expectToken(56, TokenType::kIDENTIFIER, "fizzBuzz", 11, 0);
    expectToken(57, TokenType::kSPEC_SYMBOL, "(", 11, 8);
    expectToken(58, TokenType::kNUMBER_INT, "100", 11, 9);
    expectToken(59, TokenType::kSPEC_SYMBOL, ")", 11, 12);
}