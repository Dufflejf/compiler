#include "lexer.h"

// ���캯������ʼ���ʷ�������
Lexer::Lexer() {
    initKeywords();  // ��ʼ���ؼ��ֱ�
}

// ��ʼ���ؼ��ֱ����ؼ����ַ���ӳ�䵽��Ӧ��Token����
void Lexer::initKeywords() {
    keywords["if"] = SY_IF;     // if���
    keywords["then"] = SY_THEN;  // then�Ӿ�
    keywords["else"] = SY_ELSE;  // else�Ӿ�
    keywords["while"] = SY_WHILE; // whileѭ��
    keywords["begin"] = SY_BEGIN; // ����鿪ʼ
    keywords["do"] = SY_DO;      // do-whileѭ��
    keywords["end"] = SY_END;    // ��������
    keywords["and"] = OP_AND;    // �߼���
    keywords["or"] = OP_OR;      // �߼���
    keywords["not"] = OP_NOT;    // �߼���
}

// �ж��ַ��Ƿ�Ϊ����
bool Lexer::isDigit(char c) {
    return c >= '0' && c <= '9';
}

// �ж��ַ��Ƿ�Ϊ��ĸ
bool Lexer::isLetter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// �ж��ַ��Ƿ�Ϊ�հ��ַ�
bool Lexer::isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// �ʷ���������������Դ�����ַ���ת��ΪToken����
std::vector<Token> Lexer::tokenize(const std::string& source) {
    std::vector<Token> tokens;  // �洢�����Token�б�
    int line = 1;               // ��ǰ�кţ����ڴ��󱨸棩
    size_t pos = 0;             // ��ǰɨ��λ��

    while (pos < source.length()) {
        char c = source[pos];   // ��ȡ��ǰ�ַ�

        // 1. �����հ��ַ����ո��Ʊ�������еȣ�
        if (isWhitespace(c)) {
            if (c == '\n') line++;  // �������з�ʱ�����к�
            pos++;
            continue;
        }

        // 2. �����ʶ���͹ؼ��֣�����ĸ��ͷ��
        if (isLetter(c)) {
            std::string word;
            // ��ȡ�����ĵ��ʣ���ĸ��������ɣ�
            while (pos < source.length() && (isLetter(source[pos]) || isDigit(source[pos]))) {
                word += source[pos];
                pos++;
            }

            // ����Ƿ���Ԥ����Ĺؼ���
            if (keywords.find(word) != keywords.end()) {
                tokens.emplace_back(keywords[word], word, line);  // ��ӹؼ���Token
            }
            else {
                tokens.emplace_back(IDENT, word, line);  // �����ͨ��ʶ��Token
            }
            continue;
        }

        // 3. �������ֳ���
        if (isDigit(c)) {
            std::string number;
            // ��ȡ����������
            while (pos < source.length() && isDigit(source[pos])) {
                number += source[pos];
                pos++;
            }
            tokens.emplace_back(INTCONST, number, line);  // �����������Token
            continue;
        }

        // 4. ���������ַ��������
        switch (c) {
            // ���ַ������
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
            // ���⴦����~��Ϊ������ţ�TokenTypeΪ-1��ʾ������ţ�
            tokens.emplace_back(TokenType(-1), "~", line);
            pos++;
            break;

            // ˫�ַ����������Ҫǰհһ���ַ���
        case ':':
            if (pos + 1 < source.length() && source[pos + 1] == '=') {
                tokens.emplace_back(BECOMES, ":=", line);  // ��ֵ����
                pos += 2;
            }
            else {
                pos++;  // ������ð���ݲ�����
            }
            break;

            // �Ƚ������
        case '>':
            if (pos + 1 < source.length() && source[pos + 1] == '=') {
                tokens.emplace_back(ROP, ">=", line);  // ���ڵ���
                pos += 2;
            }
            else {
                tokens.emplace_back(ROP, ">", line);   // ����
                pos++;
            }
            break;
        case '<':
            if (pos + 1 < source.length() && source[pos + 1] == '=') {
                tokens.emplace_back(ROP, "<=", line);  // С�ڵ���
                pos += 2;
            }
            else {
                tokens.emplace_back(ROP, "<", line);   // С��
                pos++;
            }
            break;
        case '=':
            tokens.emplace_back(ROP, "=", line);       // ����
            pos++;
            break;

            // Ĭ������������޷�ʶ����ַ�
        default:
            pos++;
            break;
        }
    }

    return tokens;  // �������ɵ�Token����
}