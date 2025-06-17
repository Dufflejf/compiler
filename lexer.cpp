#include "lexer.h"

Lexer::Lexer() {
    initKeywords();
}

void Lexer::initKeywords() {
    keywords["if"] = SY_IF;
    keywords["then"] = SY_THEN;
    keywords["else"] = SY_ELSE;
    keywords["while"] = SY_WHILE;
    keywords["begin"] = SY_BEGIN;
    keywords["do"] = SY_DO;
    keywords["end"] = SY_END;
    keywords["and"] = OP_AND;
    keywords["or"] = OP_OR;
    keywords["not"] = OP_NOT;
}

bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

bool Lexer::isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Lexer::isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

std::vector<Token> Lexer::tokenize(const std::string& source) {
    std::vector<Token> tokens;
    int line = 1;
    size_t pos = 0;

    while (pos < source.length()) {
        char c = source[pos];

        // �����հ��ַ�
        if (isWhitespace(c)) {
            if (c == '\n') line++;
            pos++;
            continue;
        }

        // �����ʶ���͹ؼ���
        if (isLetter(c)) {
            std::string word;
            while (pos < source.length() && (isLetter(source[pos]) || isDigit(source[pos]))) {
                word += source[pos];
                pos++;
            }

            // ����Ƿ��ǹؼ���
            if (keywords.find(word) != keywords.end()) {
                tokens.emplace_back(keywords[word], word, line);
            }
            else {
                tokens.emplace_back(IDENT, word, line);
            }
            continue;
        }

        // ��������
        if (isDigit(c)) {
            std::string number;
            while (pos < source.length() && isDigit(source[pos])) {
                number += source[pos];
                pos++;
            }
            tokens.emplace_back(INTCONST, number, line);
            continue;
        }

        // ���������ַ��������
        switch (c) {
        case '+':
            tokens.emplace_back(PLUS, "+", line);
            pos++;
            break;
        case '*':
            tokens.emplace_back(TIMES, "*", line);
            pos++;
            break;
        case '(':
            tokens.emplace_back(LPARENT, "(", line);
            pos++;
            break;
        case ')':
            tokens.emplace_back(RPARENT, ")", line);
            pos++;
            break;
        case ';':
            tokens.emplace_back(SEMICOLON, ";", line);
            pos++;
            break;
        case '#':
            tokens.emplace_back(JINGHAO, "#", line);
            pos++;
            break;
        case '~':
            // ֱ�ӽ� ~ ��Ϊ�ַ���ֵ�洢
            tokens.emplace_back(TokenType(-1), "~", line);
            pos++;
            break;
        case ':':
            if (pos + 1 < source.length() && source[pos + 1] == '=') {
                tokens.emplace_back(BECOMES, ":=", line);
                pos += 2;
            }
            else {
                pos++;
            }
            break;
        case '>':
            if (pos + 1 < source.length() && source[pos + 1] == '=') {
                tokens.emplace_back(ROP, ">=", line);
                pos += 2;
            }
            else {
                tokens.emplace_back(ROP, ">", line);
                pos++;
            }
            break;
        case '<':
            if (pos + 1 < source.length() && source[pos + 1] == '=') {
                tokens.emplace_back(ROP, "<=", line);
                pos += 2;
            }
            else {
                tokens.emplace_back(ROP, "<", line);
                pos++;
            }
            break;
        case '=':
            tokens.emplace_back(ROP, "=", line);
            pos++;
            break;
        default:
            // ����δʶ����ַ�
            pos++;
            break;
        }
    }

    return tokens;
}