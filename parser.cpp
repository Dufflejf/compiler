#include "parser.h"
#include <iostream>
#include <sstream>

// 构造函数，初始化解析器状态
Parser::Parser() : ast(nullptr), currentStatement(nullptr),
tempVarCounter(1), quadIndex(100) {//四元式索引初始100
    stateStack.push(0);  // 初始状态压栈
}

// 析构函数，清理AST内存
Parser::~Parser() {
    if (ast) {
        delete ast;
        ast = nullptr;
    }
}

// 生成四元式，赋值语句
void Parser::generateQuadruple(const std::string& op,const std::string& arg1,const std::string& arg2,const std::string& result) {
    std::stringstream ss;
    ss << quadIndex << " (";  // 四元式编号
    if (op == "+" || op == "*" || op == ":=") {  // 处理算术和赋值操作
        ss << op << "," << arg1 << ", " << arg2 << ", " << result;
    }
    ss << ")";
    quadruples.push_back(ss.str());  // 添加到四元式列表
    quadIndex++;  // 递增四元式索引
}

// 生成跳转四元式
void Parser::generateJump(const std::string& op, const std::string& arg1,const std::string& arg2,int target) {
    std::stringstream ss;
    ss << quadIndex << " (j" << op << ", " << arg1 << ", " << arg2 << ", " << target << ")";
    quadruples.push_back(ss.str());
    quadIndex++;
}

// 回填跳转地址
void Parser::backPatch(int jumpInstr, int target) {
    if (jumpInstr >= 100 && jumpInstr < quadIndex) {  // 检查跳转指令是否有效
        std::stringstream ss;
        ss << jumpInstr << " (j, , , " << target << ")";
        quadruples[jumpInstr - 100] = ss.str();  // 更新跳转目标
    }
}

// 主解析函数
bool Parser::parse(const std::vector<Token>& tokens) {
    size_t pos = 0;
    try {
        std::cout << "\n开始语法分析..." << std::endl;
        //循环处理token序列
        while (pos < tokens.size()) {
            const Token& token = tokens[pos];
            std::cout << "处理词法单元: " << getTokenInfo(token) << std::endl;
            //根据Token类型调用相应的解析函数
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

// 解析复合语句
bool Parser::parseCompoundStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析复合语句开始" << std::endl;

    // 检查begin关键字
    if (tokens[pos].type != SY_BEGIN) {
        reportError("缺少begin关键字", tokens[pos]);
        return false;
    }
    pos++;

    // 创建复合语句节点
    Node* compoundNode = new Node("Compound");
    if (!currentStatement) {
        ast = compoundNode;  // 如果是第一个语句，设为AST根节点，表示一个语句块
    }
    else {
        currentStatement->children.push_back(compoundNode);//新节点添加为当前节点的子节点
    }
    Node* previousStatement = currentStatement;//切换上下文
    currentStatement = compoundNode;//换到新创建的复合语句节点

    // 解析语句序列，直到遇到end
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

    // 检查end关键字
    if (pos >= tokens.size() || tokens[pos].type != SY_END) {
        reportError("缺少end关键字", tokens[pos]);
        return false;
    }
    pos++;

    currentStatement = previousStatement;  // 恢复当前语句指针
    std::cout << "复合语句解析完成" << std::endl;
    return true;
}

// 解析通用语句
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

// 解析if语句
bool Parser::parseIfStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析if语句开始" << std::endl;

    pos++; // 跳过if

    // 解析布尔表达式
    if (!parseBooleanExpression(tokens, pos)) {
        return false;
    }

    // 生成条件跳转，跳转到else部分
    int falseJump = quadIndex;
    generateJump("", "", "", 0);  // 直接跳转，0先占位，后面回填

    // 检查then关键字
    if (pos >= tokens.size() || tokens[pos].type != SY_THEN) {
        reportError("缺少then关键字", tokens[pos]);
        return false;
    }
    pos++;

    // 解析then部分
    if (!parseStatement(tokens, pos)) {
        return false;
    }

    // 跳过else部分的跳转
    int skipElseJump = quadIndex;
    generateJump("", "", "", 0);  // 先占位，后面回填

    // 回填条件为假时的跳转地址
    backPatch(falseJump, quadIndex);

    // 检查else关键字
    if (pos < tokens.size() && tokens[pos].type == SY_ELSE) {
        pos++;
        if (!parseStatement(tokens, pos)) {
            return false;
        }
    }

    // 回填跳过else部分的跳转地址
    backPatch(skipElseJump, quadIndex);

    return true;
}

// 解析while语句
bool Parser::parseWhileStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析while语句开始" << std::endl;

    int startLabel = quadIndex;  // 循环开始位置，用于后续生成跳回指令
    pos++; // 跳过while

    // 处理条件
    if (pos < tokens.size() && tokens[pos].type == LPARENT) {
        pos++; // 跳过左括号
    }

    // 解析布尔表达式
    if (!parseBooleanExpression(tokens, pos)) {
        return false;
    }

    if (pos < tokens.size() && tokens[pos].type == RPARENT) {
        pos++; // 跳过右括号
    }

    // 生成条件跳转
    int condJump = quadIndex;//记录条件跳转指令的位置(用于后续回填)
    int elseLabel = quadIndex + 2;  // 预估的循环体结束位置(实际可能不同)
    generateJump("", "", "", elseLabel);

    // 检查do关键字
    if (pos >= tokens.size() || tokens[pos].type != SY_DO) {
        reportError("缺少do关键字", tokens[pos]);
        return false;
    }
    pos++;

    // 解析循环体
    if (!parseStatement(tokens, pos)) {
        return false;
    }

    // 生成循环跳转回开始
    generateJump("", "", "", startLabel);

    // 设置循环结束的位置
    int endLabel = quadIndex;

    // 将之前生成的占位跳转指令的目标地址填充为endLabel
    backPatch(condJump, endLabel);

    return true;
}

// 解析赋值语句
bool Parser::parseAssignmentStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析赋值语句开始" << std::endl;

    // 保存并检查左侧标识符
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

    // 解析右侧表达式
    if (!parseExpression(tokens, pos)) {
        return false;
    }

    // 生成赋值四元式
    generateQuadruple(":=", expressionResult, "", identifier);
    std::cout << "赋值语句解析完成" << std::endl;
    return true;
}

// 解析表达式，递归下降解析器
bool Parser::parseExpression(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析表达式开始" << std::endl;

    if (!parseTerm(tokens, pos)) {
        return false;
    }
    std::string leftOperand = expressionResult;//保存左操作数

    // 处理加法运算
    while (pos < tokens.size() && tokens[pos].type == PLUS) {
        pos++; // 跳过加号
        //
        if (!parseTerm(tokens, pos)) {//解析右操作数
            return false;
        }
        std::string rightOperand = expressionResult;//保存右操作数结果

        // 生成临时变量存储结果
        std::string result = "T" + std::to_string(tempVarCounter++);
        generateQuadruple("+", leftOperand, rightOperand, result);//生成四元式
        expressionResult = result;//更新结果
        leftOperand = result;
    }

    // 处理乘法运算
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

    std::cout << "表达式解析完成" << std::endl;
    return true;
}

// 解析项
bool Parser::parseTerm(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析项开始" << std::endl;

    if (!parseFactor(tokens, pos)) {
        return false;
    }

    // 处理加法运算
    while (pos < tokens.size() && tokens[pos].type == PLUS) {
        pos++; // 跳过加号
        std::string leftOperand = expressionResult;//保存左操作数
        if (!parseTerm(tokens, pos)) {//解析右操作数
            return false;
        }
        std::string rightOperand = expressionResult;//保存右操作数

        // 生成临时变量存储结果
        std::string result = "T" + std::to_string(tempVarCounter++);
        generateQuadruple("+", leftOperand, rightOperand, result);//生成四元式
        expressionResult = result;//更新结果
        leftOperand = result;//以支持连续计算
    }

    // 处理乘法运算
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

// 解析因子
bool Parser::parseFactor(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析因子开始" << std::endl;

    if (pos >= tokens.size()) {//判断是否超过token长度，如+后无内容
        Token invalidToken;
        reportError("表达式非法结束", invalidToken);
        return false;
    }

    const Token& token = tokens[pos];
    switch (token.type) {
    case IDENT:  // 标识符
    case INTCONST:  // 整型常量
        expressionResult = token.value;
        pos++;
        break;

    case LPARENT:  // 括号表达式
        pos++; // 跳过左括号
        if (!parseExpression(tokens, pos)) {//解析表达式
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

// 解析带括号的布尔表达式
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

// 解析布尔表达式
bool Parser::parseBooleanExpression(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "解析布尔表达式开始" << std::endl;

    // 解析左操作数
    if (!parseExpression(tokens, pos)) {
        return false;
    }
    std::string leftOperand = expressionResult;//保存左操作数

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
    std::string rightOperand = expressionResult;//保存右操作数

    // 生成条件跳转指令
    int nextQuad = quadIndex + 2;  // 跳过紧跟着的无条件跳转
    generateJump(op, leftOperand, rightOperand, nextQuad);

    return true;
}

// 获取生成的四元式列表
std::vector<std::string> Parser::getQuadruples() const {
    return quadruples;
}

// 获取抽象语法树
Node* Parser::getAST() const {
    return ast;
}

// 获取词法单元信息字符串
std::string Parser::getTokenInfo(const Token& token) {
    std::stringstream ss;
    ss << "类型: " << tokenTypeToString(token.type)//序号转成字符串
        << ", 值: " << token.value
        << ", 行号: " << token.line;
    return ss.str();
}

// 报告错误信息
void Parser::reportError(const std::string& message, const Token& token) {
    std::cerr << "语法错误: " << message << " ";
    if (token.type != TokenType(-1)) {
        std::cerr << "在 " << getTokenInfo(token);
    }
    std::cerr << std::endl;
}

// 将词法单元类型转换为字符串表示
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