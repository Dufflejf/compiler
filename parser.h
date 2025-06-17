#pragma once
#include "lexer.h"
#include <vector>
#include <stack>
#include <string>

struct Node {
    std::string type;
    std::string value;
    std::vector<Node*> children;

    Node(const std::string& t, const std::string& v = "")
        : type(t), value(v) {
    }

    ~Node() {
        for (auto child : children) {
            delete child;
        }
    }
};

class Parser {
public:
    Parser();
    ~Parser();
    bool parse(const std::vector<Token>& tokens);
    std::vector<std::string> getQuadruples() const;
    Node* getAST() const;

private:
    std::stack<int> stateStack;
    std::stack<std::string> symbolStack;
    std::vector<std::string> quadruples;
    Node* ast;
    Node* currentStatement;
    int tempVarCounter;
    int quadIndex;  // ��Ԫʽ���
    std::map<std::string, int> labelMap;  // ��ǩӳ��
    std::string expressionResult;
    bool parseParenBooleanExpression(const std::vector<Token>& tokens, size_t& pos);
    bool parseStatement(const std::vector<Token>& tokens, size_t& pos);
    bool parseCompoundStatement(const std::vector<Token>& tokens, size_t& pos);
    bool parseIfStatement(const std::vector<Token>& tokens, size_t& pos);
    bool parseWhileStatement(const std::vector<Token>& tokens, size_t& pos);
    bool parseAssignmentStatement(const std::vector<Token>& tokens, size_t& pos);
    bool parseExpression(const std::vector<Token>& tokens, size_t& pos);
    bool parseTerm(const std::vector<Token>& tokens, size_t& pos);
    bool parseFactor(const std::vector<Token>& tokens, size_t& pos);
    bool parseBooleanExpression(const std::vector<Token>& tokens, size_t& pos);
    void generateQuadruple(const std::string& op,
        const std::string& arg1,
        const std::string& arg2,
        const std::string& result);
    std::string getTokenInfo(const Token& token);
    void reportError(const std::string& message, const Token& token);
    std::string tokenTypeToString(TokenType type);
    
};