#include "lexer.h"

// 构造函数：初始化词法分析器
Lexer::Lexer() {
    initKeywords();  // 初始化关键字表
}

// 初始化关键字表：将关键字字符串映射到对应的Token类型
void Lexer::initKeywords() {
    keywords["if"] = SY_IF;     // if语句
    keywords["then"] = SY_THEN;  // then子句
    keywords["else"] = SY_ELSE;  // else子句
    keywords["while"] = SY_WHILE; // while循环
    keywords["begin"] = SY_BEGIN; // 代码块开始
    keywords["do"] = SY_DO;      // do-while循环
    keywords["end"] = SY_END;    // 代码块结束
    keywords["and"] = OP_AND;    // 逻辑与
    keywords["or"] = OP_OR;      // 逻辑或
    keywords["not"] = OP_NOT;    // 逻辑非
}

// 判断字符是否为数字
bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

// 判断字符是否为字母
bool Lexer::isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// 判断字符是否为空白字符
bool Lexer::isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// 词法分析主函数：将源代码字符串转换为Token序列
std::vector<Token> Lexer::tokenize(const std::string& source) {
    std::vector<Token> tokens;  // 存储结果的Token列表
    int line = 1;               // 当前行号（用于错误报告）
    size_t pos = 0;             // 当前扫描位置

    while (pos < source.length()) {
        char c = source[pos];   // 获取当前字符

        // 1. 跳过空白字符（空格、制表符、换行等）
        if (isWhitespace(c)) {
            if (c == '\n') line++;  // 遇到换行符时增加行号
            pos++;
            continue;
        }

        // 2. 处理标识符和关键字（以字母开头）
        if (isLetter(c)) {
            std::string word;
            // 读取完整的单词（字母或数字组成）
            while (pos < source.length() && (isLetter(source[pos]) || isDigit(source[pos]))) {
                word += source[pos];
                pos++;
            }

            // 检查是否是预定义的关键字
            if (keywords.find(word) != keywords.end()) {
                tokens.emplace_back(keywords[word], word, line);  // 添加关键字Token
            }
            else {
                tokens.emplace_back(IDENT, word, line);  // 添加普通标识符Token
            }
            continue;
        }

        // 3. 处理数字常量
        if (isDigit(c)) {
            std::string number;
            // 读取连续的数字
            while (pos < source.length() && isDigit(source[pos])) {
                number += source[pos];
                pos++;
            }
            tokens.emplace_back(INTCONST, number, line);  // 添加整数常量Token
            continue;
        }

        // 4. 处理特殊字符和运算符
        switch (c) {
            // 单字符运算符
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
            // 特殊处理：将~作为特殊符号（TokenType为-1表示特殊符号）
            tokens.emplace_back(TokenType(-1), "~", line);
            pos++;
            break;

            // 双字符运算符（需要前瞻一个字符）
        case ':':
            if (pos + 1 < source.length() && source[pos + 1] == '=') {
                tokens.emplace_back(BECOMES, ":=", line);  // 赋值符号
                pos += 2;
            }
            else {
                pos++;  // 单独的冒号暂不处理
            }
            break;

            // 比较运算符
        case '>':
            if (pos + 1 < source.length() && source[pos + 1] == '=') {
                tokens.emplace_back(ROP, ">=", line);  // 大于等于
                pos += 2;
            }
            else {
                tokens.emplace_back(ROP, ">", line);   // 大于
                pos++;
            }
            break;
        case '<':
            if (pos + 1 < source.length() && source[pos + 1] == '=') {
                tokens.emplace_back(ROP, "<=", line);  // 小于等于
                pos += 2;
            }
            else {
                tokens.emplace_back(ROP, "<", line);   // 小于
                pos++;
            }
            break;
        case '=':
            tokens.emplace_back(ROP, "=", line);       // 等于
            pos++;
            break;

            // 默认情况：忽略无法识别的字符
        default:
            pos++;
            break;
        }
    }

    return tokens;  // 返回生成的Token序列
}