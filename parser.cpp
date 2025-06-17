#include "parser.h"
#include <iostream>
#include <sstream>

Parser::Parser() : ast(nullptr), currentStatement(nullptr), tempVarCounter(0) {
    stateStack.push(0);
}

Parser::~Parser() {
    if (ast) {
        delete ast;
        ast = nullptr;
    }
}

bool Parser::parse(const std::vector<Token>& tokens) {
    size_t pos = 0;
    try {
        std::cout << "\n��ʼ�﷨����..." << std::endl;

        while (pos < tokens.size()) {
            const Token& token = tokens[pos];
            std::cout << "����ʷ���Ԫ: " << getTokenInfo(token) << std::endl;

            switch (token.type) {
            case SY_BEGIN:
                if (!parseCompoundStatement(tokens, pos)) {
                    reportError("����������ʧ��", token);
                    return false;
                }
                break;

            case SY_WHILE:
                if (!parseWhileStatement(tokens, pos)) {
                    reportError("while������ʧ��", token);
                    return false;
                }
                break;

            case SY_IF:
                if (!parseIfStatement(tokens, pos)) {
                    reportError("if������ʧ��", token);
                    return false;
                }
                break;

            case IDENT:
                if (!parseAssignmentStatement(tokens, pos)) {
                    reportError("��ֵ������ʧ��", token);
                    return false;
                }
                break;

            case JINGHAO:
                // ������������ "#~"
                pos++; // ���� #
                if (pos < tokens.size() && tokens[pos].value == "~") {
                    std::cout << "���ֳ��������� #~" << std::endl;
                    pos++; // ���� ~
                    std::cout << "����������" << std::endl;
                    return true;
                }
                reportError("�������ĳ��������ǣ���Ҫ #~", token);
                return false;

            default:
                reportError("����Ĵʷ���Ԫ", token);
                return false;
            }
        }

        // ����������ﵫû������ #~��Ҳ�������
        Token invalidToken;
        reportError("ȱ�ٳ��������� #~", invalidToken);
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "�������̷����쳣: " << e.what() << std::endl;
        return false;
    }
}

bool Parser::parseCompoundStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "����������俪ʼ" << std::endl;

    // ���begin
    if (tokens[pos].type != SY_BEGIN) {
        reportError("ȱ��begin�ؼ���", tokens[pos]);
        return false;
    }
    pos++;

    Node* compoundNode = new Node("Compound");
    if (!currentStatement) {
        ast = compoundNode;
    }
    else {
        currentStatement->children.push_back(compoundNode);
    }
    Node* previousStatement = currentStatement;
    currentStatement = compoundNode;

    // �����������
    while (pos < tokens.size() && tokens[pos].type != SY_END) {
        switch (tokens[pos].type) {
        case SY_IF:
            if (!parseIfStatement(tokens, pos)) return false;
            break;
        case SY_WHILE:
            if (!parseWhileStatement(tokens, pos)) return false;
            break;
        case IDENT:
            if (!parseAssignmentStatement(tokens, pos)) return false;
            break;
        case SEMICOLON:
            pos++;
            continue;
        default:
            if (tokens[pos].type != SY_END) { // ��Ҫ��end����Ϊ����
                reportError("��Ч����俪ʼ", tokens[pos]);
                return false;
            }
        }

        // �������ָ���
        if (pos < tokens.size() && tokens[pos].type == SEMICOLON) {
            pos++;
        }
    }

    // ���end
    if (pos >= tokens.size() || tokens[pos].type != SY_END) {
        reportError("ȱ��end�ؼ���", tokens[pos]);
        return false;
    }
    pos++;

    currentStatement = previousStatement;
    std::cout << "�������������" << std::endl;
    return true;
}

bool Parser::parseStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "������俪ʼ" << std::endl;

    if (pos >= tokens.size()) {
        Token invalidToken;
        reportError("�����������", invalidToken);
        return false;
    }

    switch (tokens[pos].type) {
    case SY_IF:
        return parseIfStatement(tokens, pos);

    case SY_WHILE:
        return parseWhileStatement(tokens, pos);

    case SY_BEGIN:
        return parseCompoundStatement(tokens, pos);

    case IDENT:
        return parseAssignmentStatement(tokens, pos);

    default:
        reportError("��Ч�����", tokens[pos]);
        return false;
    }
}

bool Parser::parseIfStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "����if��俪ʼ" << std::endl;

    pos++; // ����if

    Node* ifNode = new Node("If");
    if (!currentStatement) {
        ast = ifNode;
    }
    else {
        currentStatement->children.push_back(ifNode);
    }
    Node* previousStatement = currentStatement;
    currentStatement = ifNode;

    // ֱ�ӽ����������ʽ������Ҫ���ţ�
    if (!parseBooleanExpression(tokens, pos)) {
        delete ifNode;
        return false;
    }

    // ���then
    if (pos >= tokens.size() || tokens[pos].type != SY_THEN) {
        reportError("ȱ��then�ؼ���", tokens[pos]);
        delete ifNode;
        return false;
    }
    pos++;

    // ����then����
    if (!parseStatement(tokens, pos)) {
        delete ifNode;
        return false;
    }

    // ���else
    if (pos < tokens.size() && tokens[pos].type == SY_ELSE) {
        pos++;
        if (!parseStatement(tokens, pos)) {
            delete ifNode;
            return false;
        }
    }

    currentStatement = previousStatement;
    std::cout << "if���������" << std::endl;
    return true;
}

bool Parser::parseWhileStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "����while��俪ʼ" << std::endl;
    pos++; // ����while

    Node* whileNode = new Node("While");
    if (!currentStatement) {
        ast = whileNode;
    }
    else {
        currentStatement->children.push_back(whileNode);
    }
    Node* previousStatement = currentStatement;
    currentStatement = whileNode;

    // ����Ƿ���������
    bool hasParentheses = (pos < tokens.size() && tokens[pos].type == LPARENT);

    if (hasParentheses) {
        pos++; // ����������
    }

    // �����������ʽ
    if (!parseBooleanExpression(tokens, pos)) {
        delete whileNode;
        return false;
    }

    if (hasParentheses) {
        // ����������ţ�������������
        if (pos >= tokens.size() || tokens[pos].type != RPARENT) {
            reportError("ȱ��������", tokens[pos]);
            delete whileNode;
            return false;
        }
        pos++; // ����������
    }

    // ���do
    if (pos >= tokens.size() || tokens[pos].type != SY_DO) {
        reportError("ȱ��do�ؼ���", tokens[pos]);
        delete whileNode;
        return false;
    }
    pos++;

    // ����ѭ����
    if (!parseStatement(tokens, pos)) {
        delete whileNode;
        return false;
    }

    currentStatement = previousStatement;
    std::cout << "while���������" << std::endl;
    return true;
}

bool Parser::parseAssignmentStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "������ֵ��俪ʼ" << std::endl;

    // �����ʶ��
    if (pos >= tokens.size() || tokens[pos].type != IDENT) {
        reportError("ȱ�ٱ�ʶ��", tokens[pos]);
        return false;
    }
    std::string identifier = tokens[pos].value;
    pos++;

    // ��鸳ֵ����
    if (pos >= tokens.size() || tokens[pos].type != BECOMES) {
        reportError("ȱ�ٸ�ֵ����", tokens[pos]);
        return false;
    }
    pos++;

    // �������ʽ
    if (!parseExpression(tokens, pos)) {
        return false;
    }

    // ���ɸ�ֵ��Ԫʽ
    generateQuadruple(":=", expressionResult, "", identifier);
    std::cout << "��ֵ���������" << std::endl;
    return true;
}

bool Parser::parseExpression(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "�������ʽ��ʼ" << std::endl;

    if (!parseTerm(tokens, pos)) {
        return false;
    }
    std::string leftOperand = expressionResult;

    while (pos < tokens.size() && tokens[pos].type == PLUS) {
        pos++; // �����Ӻ�

        if (!parseTerm(tokens, pos)) {
            return false;
        }
        std::string rightOperand = expressionResult;

        std::string result = "T" + std::to_string(tempVarCounter++);
        generateQuadruple("+", leftOperand, rightOperand, result);
        expressionResult = result;
        leftOperand = result;
    }

    std::cout << "���ʽ�������" << std::endl;
    return true;
}

bool Parser::parseTerm(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "�����ʼ" << std::endl;

    if (!parseFactor(tokens, pos)) {
        return false;
    }

    while (pos < tokens.size() && tokens[pos].type == TIMES) {
        pos++; // �����˺�
        std::string leftOperand = expressionResult;

        if (!parseFactor(tokens, pos)) {
            return false;
        }

        std::string rightOperand = expressionResult;
        std::string result = "T" + std::to_string(tempVarCounter++);
        generateQuadruple("*", leftOperand, rightOperand, result);
        expressionResult = result;
    }

    std::cout << "��������" << std::endl;
    return true;
}

bool Parser::parseFactor(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "�������ӿ�ʼ" << std::endl;

    if (pos >= tokens.size()) {
        Token invalidToken;
        reportError("���ʽ�Ƿ�����", invalidToken);
        return false;
    }

    const Token& token = tokens[pos];
    switch (token.type) {
    case IDENT:
    case INTCONST:
        expressionResult = token.value;
        pos++;
        break;

    case LPARENT:
        pos++; // ����������
        if (!parseExpression(tokens, pos)) {
            return false;
        }
        if (pos >= tokens.size() || tokens[pos].type != RPARENT) {
            reportError("ȱ��������", tokens[pos]);
            return false;
        }
        pos++; // ����������
        break;

    default:
        reportError("�Ƿ�������", token);
        return false;
    }

    std::cout << "���ӽ������" << std::endl;
    return true;
}

bool Parser::parseParenBooleanExpression(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "���������ŵĲ������ʽ��ʼ" << std::endl;

    // ���������
    if (pos >= tokens.size() || tokens[pos].type != LPARENT) {
        reportError("ȱ��������", tokens[pos]);
        return false;
    }
    pos++; // ����������

    // �����������ʽ����
    if (!parseBooleanExpression(tokens, pos)) {
        return false;
    }

    // ���������
    if (pos >= tokens.size() || tokens[pos].type != RPARENT) {
        reportError("ȱ��������", tokens[pos]);
        return false;
    }
    pos++; // ����������

    return true;
}

bool Parser::parseBooleanExpression(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "�����������ʽ��ʼ" << std::endl;

    // �����������
    if (!parseExpression(tokens, pos)) {
        return false;
    }
    std::string leftOperand = expressionResult;

    // ��ȡ��ϵ�����
    if (pos >= tokens.size() || tokens[pos].type != ROP) {
        reportError("ȱ�ٹ�ϵ�����", tokens[pos]);
        return false;
    }
    std::string op = tokens[pos].value;
    pos++;

    // �����Ҳ�����
    if (!parseExpression(tokens, pos)) {
        return false;
    }
    std::string rightOperand = expressionResult;

    // ���ɲ������ʽ����Ԫʽ
    std::string result = "B" + std::to_string(tempVarCounter++);
    generateQuadruple(op, leftOperand, rightOperand, result);
    expressionResult = result;

    std::cout << "�������ʽ�������" << std::endl;
    return true;
}

void Parser::generateQuadruple(const std::string& op,
    const std::string& arg1,
    const std::string& arg2,
    const std::string& result) {
    std::stringstream ss;
    ss << "(" << op << ", " << arg1 << ", " << arg2 << ", " << result << ")";
    quadruples.push_back(ss.str());
    std::cout << "������Ԫʽ: " << ss.str() << std::endl;
}

std::vector<std::string> Parser::getQuadruples() const {
    return quadruples;
}

Node* Parser::getAST() const {
    return ast;
}

std::string Parser::getTokenInfo(const Token& token) {
    std::stringstream ss;
    ss << "����: " << tokenTypeToString(token.type)
        << ", ֵ: " << token.value
        << ", �к�: " << token.line;
    return ss.str();
}

void Parser::reportError(const std::string& message, const Token& token) {
    std::cerr << "�﷨����: " << message << " ";
    if (token.type != TokenType(-1)) {
        std::cerr << "�� " << getTokenInfo(token);
    }
    std::cerr << std::endl;
}

std::string Parser::tokenTypeToString(TokenType type) {
    switch (type) {
    case SY_IF: return "IF";
    case SY_THEN: return "THEN";
    case SY_ELSE: return "ELSE";
    case SY_WHILE: return "WHILE";
    case SY_BEGIN: return "BEGIN";
    case SY_DO: return "DO";
    case SY_END: return "END";
    case BECOMES: return "BECOMES";
    case SEMICOLON: return "SEMICOLON";
    case JINGHAO: return "JINGHAO";
    case PLUS: return "PLUS";
    case TIMES: return "TIMES";
    case ROP: return "ROP";
    case LPARENT: return "LPARENT";
    case RPARENT: return "RPARENT";
    case IDENT: return "IDENT";
    case INTCONST: return "INTCONST";
    default: return "UNKNOWN";
    }
}