#pragma once
#include <string>
#include <vector>
#include <map>

enum TokenType {
    SY_IF = 0,
    SY_THEN = 1,
    SY_ELSE = 2,
    SY_WHILE = 3,
    SY_BEGIN = 4,
    SY_DO = 5,
    SY_END = 6,
    A = 7,
    SEMICOLON = 8,
    E = 9,
    JINGHAO = 10,
    S = 11,
    L = 12,
    TEMPSY = 15,
    EA = 18,
    EO = 19,
    PLUS = 34,
    TIMES = 36,
    BECOMES = 38,
    OP_AND = 39,
    OP_OR = 40,
    OP_NOT = 41,
    ROP = 42,
    LPARENT = 48,
    RPARENT = 49,
    IDENT = 56,
    INTCONST = 57
};

// Token结构定义
struct Token {
    TokenType type;
    std::string value;
    int line;

    Token() : type(TokenType(-1)), value(""), line(-1) {}
    Token(TokenType t, const std::string& v, int l)
        : type(t), value(v), line(l) {
    }
};

class Lexer {
public:
    Lexer();
    std::vector<Token> tokenize(const std::string& source);

private:
    std::map<std::string, TokenType> keywords;
    void initKeywords();
    bool isDigit(char c);
    bool isLetter(char c);
    bool isWhitespace(char c);
};
