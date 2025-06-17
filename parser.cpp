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
        std::cout << "\n开始语法分析..." << std::endl;

        while (pos < tokens.size()) {
            const Token& token = tokens[pos];
            std::cout << "处理词法单元: " << getTokenInfo(token) << std::endl;

            switch (token.type) {
            case SY_BEGIN:
                if (!parseCompoundStatement(tokens, pos)) {
                    reportError("复合语句解析失败", token);
                    return false;
                }
                break;

            case SY_WHILE:
                if (!parseWhileStatement(tokens, pos)) {
                    reportError("while语句解析失败", token);
                    return false;
                }
                break;

            case SY_IF:
                if (!parseIfStatement(tokens, pos)) {
                    reportError("if语句解析失败", token);
                    return false;
                }
                break;

            case IDENT:
                if (!parseAssignmentStatement(tokens, pos)) {
                    reportError("赋值语句解析失败", token);
                    return false;
                }
                break;

            case JINGHAO:
                // 检查程序结束标记 "#~"
                pos++; // 跳过 #
                if (pos < tokens.size() && tokens[pos].value == "~") {
                    std::cout << "发现程序结束标记 #~" << std::endl;
                    pos++; // 跳过 ~
                    std::cout << "程序解析完成" << std::endl;
                    return true;
                }
                reportError("不完整的程序结束标记，需要 #~", token);
                return false;

            default:
                reportError("意外的词法单元", token);
                return false;
            }
        }

        // 如果到达这里但没有遇到 #~，也报告错误
        Token invalidToken;
        reportError("缺少程序结束标记 #~", invalidToken);
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "解析过程发生异常: " << e.what() << std::endl;
        return false;
    }
}

bool Parser::parseCompoundStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析复合语句开始" << std::endl;

    // 检查begin
    if (tokens[pos].type != SY_BEGIN) {
        reportError("缺少begin关键字", tokens[pos]);
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

    // 解析语句序列
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
            if (tokens[pos].type != SY_END) { // 不要把end报告为错误
                reportError("无效的语句开始", tokens[pos]);
                return false;
            }
        }

        // 处理语句分隔符
        if (pos < tokens.size() && tokens[pos].type == SEMICOLON) {
            pos++;
        }
    }

    // 检查end
    if (pos >= tokens.size() || tokens[pos].type != SY_END) {
        reportError("缺少end关键字", tokens[pos]);
        return false;
    }
    pos++;

    currentStatement = previousStatement;
    std::cout << "复合语句解析完成" << std::endl;
    return true;
}

bool Parser::parseStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析语句开始" << std::endl;

    if (pos >= tokens.size()) {
        Token invalidToken;
        reportError("意外的语句结束", invalidToken);
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
        reportError("无效的语句", tokens[pos]);
        return false;
    }
}

bool Parser::parseIfStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析if语句开始" << std::endl;

    pos++; // 跳过if

    Node* ifNode = new Node("If");
    if (!currentStatement) {
        ast = ifNode;
    }
    else {
        currentStatement->children.push_back(ifNode);
    }
    Node* previousStatement = currentStatement;
    currentStatement = ifNode;

    // 直接解析布尔表达式（不需要括号）
    if (!parseBooleanExpression(tokens, pos)) {
        delete ifNode;
        return false;
    }

    // 检查then
    if (pos >= tokens.size() || tokens[pos].type != SY_THEN) {
        reportError("缺少then关键字", tokens[pos]);
        delete ifNode;
        return false;
    }
    pos++;

    // 解析then部分
    if (!parseStatement(tokens, pos)) {
        delete ifNode;
        return false;
    }

    // 检查else
    if (pos < tokens.size() && tokens[pos].type == SY_ELSE) {
        pos++;
        if (!parseStatement(tokens, pos)) {
            delete ifNode;
            return false;
        }
    }

    currentStatement = previousStatement;
    std::cout << "if语句解析完成" << std::endl;
    return true;
}

bool Parser::parseWhileStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析while语句开始" << std::endl;
    pos++; // 跳过while

    Node* whileNode = new Node("While");
    if (!currentStatement) {
        ast = whileNode;
    }
    else {
        currentStatement->children.push_back(whileNode);
    }
    Node* previousStatement = currentStatement;
    currentStatement = whileNode;

    // 检查是否有左括号
    bool hasParentheses = (pos < tokens.size() && tokens[pos].type == LPARENT);

    if (hasParentheses) {
        pos++; // 跳过左括号
    }

    // 解析条件表达式
    if (!parseBooleanExpression(tokens, pos)) {
        delete whileNode;
        return false;
    }

    if (hasParentheses) {
        // 如果有左括号，必须检查右括号
        if (pos >= tokens.size() || tokens[pos].type != RPARENT) {
            reportError("缺少右括号", tokens[pos]);
            delete whileNode;
            return false;
        }
        pos++; // 跳过右括号
    }

    // 检查do
    if (pos >= tokens.size() || tokens[pos].type != SY_DO) {
        reportError("缺少do关键字", tokens[pos]);
        delete whileNode;
        return false;
    }
    pos++;

    // 解析循环体
    if (!parseStatement(tokens, pos)) {
        delete whileNode;
        return false;
    }

    currentStatement = previousStatement;
    std::cout << "while语句解析完成" << std::endl;
    return true;
}

bool Parser::parseAssignmentStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析赋值语句开始" << std::endl;

    // 保存标识符
    if (pos >= tokens.size() || tokens[pos].type != IDENT) {
        reportError("缺少标识符", tokens[pos]);
        return false;
    }
    std::string identifier = tokens[pos].value;
    pos++;

    // 检查赋值符号
    if (pos >= tokens.size() || tokens[pos].type != BECOMES) {
        reportError("缺少赋值符号", tokens[pos]);
        return false;
    }
    pos++;

    // 解析表达式
    if (!parseExpression(tokens, pos)) {
        return false;
    }

    // 生成赋值四元式
    generateQuadruple(":=", expressionResult, "", identifier);
    std::cout << "赋值语句解析完成" << std::endl;
    return true;
}

bool Parser::parseExpression(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析表达式开始" << std::endl;

    if (!parseTerm(tokens, pos)) {
        return false;
    }
    std::string leftOperand = expressionResult;

    while (pos < tokens.size() && tokens[pos].type == PLUS) {
        pos++; // 跳过加号

        if (!parseTerm(tokens, pos)) {
            return false;
        }
        std::string rightOperand = expressionResult;

        std::string result = "T" + std::to_string(tempVarCounter++);
        generateQuadruple("+", leftOperand, rightOperand, result);
        expressionResult = result;
        leftOperand = result;
    }

    std::cout << "表达式解析完成" << std::endl;
    return true;
}

bool Parser::parseTerm(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析项开始" << std::endl;

    if (!parseFactor(tokens, pos)) {
        return false;
    }

    while (pos < tokens.size() && tokens[pos].type == TIMES) {
        pos++; // 跳过乘号
        std::string leftOperand = expressionResult;

        if (!parseFactor(tokens, pos)) {
            return false;
        }

        std::string rightOperand = expressionResult;
        std::string result = "T" + std::to_string(tempVarCounter++);
        generateQuadruple("*", leftOperand, rightOperand, result);
        expressionResult = result;
    }

    std::cout << "项解析完成" << std::endl;
    return true;
}

bool Parser::parseFactor(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析因子开始" << std::endl;

    if (pos >= tokens.size()) {
        Token invalidToken;
        reportError("表达式非法结束", invalidToken);
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
        pos++; // 跳过左括号
        if (!parseExpression(tokens, pos)) {
            return false;
        }
        if (pos >= tokens.size() || tokens[pos].type != RPARENT) {
            reportError("缺少右括号", tokens[pos]);
            return false;
        }
        pos++; // 跳过右括号
        break;

    default:
        reportError("非法的因子", token);
        return false;
    }

    std::cout << "因子解析完成" << std::endl;
    return true;
}

bool Parser::parseParenBooleanExpression(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析带括号的布尔表达式开始" << std::endl;

    // 检查左括号
    if (pos >= tokens.size() || tokens[pos].type != LPARENT) {
        reportError("缺少左括号", tokens[pos]);
        return false;
    }
    pos++; // 跳过左括号

    // 解析布尔表达式内容
    if (!parseBooleanExpression(tokens, pos)) {
        return false;
    }

    // 检查右括号
    if (pos >= tokens.size() || tokens[pos].type != RPARENT) {
        reportError("缺少右括号", tokens[pos]);
        return false;
    }
    pos++; // 跳过右括号

    return true;
}

bool Parser::parseBooleanExpression(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析布尔表达式开始" << std::endl;

    // 解析左操作数
    if (!parseExpression(tokens, pos)) {
        return false;
    }
    std::string leftOperand = expressionResult;

    // 获取关系运算符
    if (pos >= tokens.size() || tokens[pos].type != ROP) {
        reportError("缺少关系运算符", tokens[pos]);
        return false;
    }
    std::string op = tokens[pos].value;
    pos++;

    // 解析右操作数
    if (!parseExpression(tokens, pos)) {
        return false;
    }
    std::string rightOperand = expressionResult;

    // 生成布尔表达式的四元式
    std::string result = "B" + std::to_string(tempVarCounter++);
    generateQuadruple(op, leftOperand, rightOperand, result);
    expressionResult = result;

    std::cout << "布尔表达式解析完成" << std::endl;
    return true;
}

void Parser::generateQuadruple(const std::string& op,
    const std::string& arg1,
    const std::string& arg2,
    const std::string& result) {
    std::stringstream ss;
    ss << "(" << op << ", " << arg1 << ", " << arg2 << ", " << result << ")";
    quadruples.push_back(ss.str());
    std::cout << "生成四元式: " << ss.str() << std::endl;
}

std::vector<std::string> Parser::getQuadruples() const {
    return quadruples;
}

Node* Parser::getAST() const {
    return ast;
}

std::string Parser::getTokenInfo(const Token& token) {
    std::stringstream ss;
    ss << "类型: " << tokenTypeToString(token.type)
        << ", 值: " << token.value
        << ", 行号: " << token.line;
    return ss.str();
}

void Parser::reportError(const std::string& message, const Token& token) {
    std::cerr << "语法错误: " << message << " ";
    if (token.type != TokenType(-1)) {
        std::cerr << "在 " << getTokenInfo(token);
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