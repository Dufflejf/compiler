#include "parser.h"
#include <iostream>
#include <sstream>

// ���캯������ʼ��������״̬
Parser::Parser() : ast(nullptr), currentStatement(nullptr),
tempVarCounter(1), quadIndex(100) {//��Ԫʽ������ʼ100
    stateStack.push(0);  // ��ʼ״̬ѹջ
}

// ��������������AST�ڴ�
Parser::~Parser() {
    if (ast) {
        delete ast;
        ast = nullptr;
    }
}

// ������Ԫʽ����ֵ���
void Parser::generateQuadruple(const std::string& op,const std::string& arg1,const std::string& arg2,const std::string& result) {
    std::stringstream ss;
    ss << quadIndex << " (";  // ��Ԫʽ���
    if (op == "+" || op == "*" || op == ":=") {  // ���������͸�ֵ����
        ss << op << "," << arg1 << ", " << arg2 << ", " << result;
    }
    ss << ")";
    quadruples.push_back(ss.str());  // ��ӵ���Ԫʽ�б�
    quadIndex++;  // ������Ԫʽ����
}

// ������ת��Ԫʽ
void Parser::generateJump(const std::string& op, const std::string& arg1,const std::string& arg2,int target) {
    std::stringstream ss;
    ss << quadIndex << " (j" << op << ", " << arg1 << ", " << arg2 << ", " << target << ")";
    quadruples.push_back(ss.str());
    quadIndex++;
}

// ������ת��ַ
void Parser::backPatch(int jumpInstr, int target) {
    if (jumpInstr >= 100 && jumpInstr < quadIndex) {  // �����תָ���Ƿ���Ч
        std::stringstream ss;
        ss << jumpInstr << " (j, , , " << target << ")";
        quadruples[jumpInstr - 100] = ss.str();  // ������תĿ��
    }
}

// ����������
bool Parser::parse(const std::vector<Token>& tokens) {
    size_t pos = 0;
    try {
        std::cout << "\n��ʼ�﷨����..." << std::endl;
        //ѭ������token����
        while (pos < tokens.size()) {
            const Token& token = tokens[pos];
            std::cout << "����ʷ���Ԫ: " << getTokenInfo(token) << std::endl;
            //����Token���͵�����Ӧ�Ľ�������
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

// �����������
bool Parser::parseCompoundStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "����������俪ʼ" << std::endl;

    // ���begin�ؼ���
    if (tokens[pos].type != SY_BEGIN) {
        reportError("ȱ��begin�ؼ���", tokens[pos]);
        return false;
    }
    pos++;

    // �����������ڵ�
    Node* compoundNode = new Node("Compound");
    if (!currentStatement) {
        ast = compoundNode;  // ����ǵ�һ����䣬��ΪAST���ڵ㣬��ʾһ������
    }
    else {
        currentStatement->children.push_back(compoundNode);//�½ڵ����Ϊ��ǰ�ڵ���ӽڵ�
    }
    Node* previousStatement = currentStatement;//�л�������
    currentStatement = compoundNode;//�����´����ĸ������ڵ�

    // ����������У�ֱ������end
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

    // ���end�ؼ���
    if (pos >= tokens.size() || tokens[pos].type != SY_END) {
        reportError("ȱ��end�ؼ���", tokens[pos]);
        return false;
    }
    pos++;

    currentStatement = previousStatement;  // �ָ���ǰ���ָ��
    std::cout << "�������������" << std::endl;
    return true;
}

// ����ͨ�����
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

// ����if���
bool Parser::parseIfStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "����if��俪ʼ" << std::endl;

    pos++; // ����if

    // �����������ʽ
    if (!parseBooleanExpression(tokens, pos)) {
        return false;
    }

    // ����������ת����ת��else����
    int falseJump = quadIndex;
    generateJump("", "", "", 0);  // ֱ����ת��0��ռλ���������

    // ���then�ؼ���
    if (pos >= tokens.size() || tokens[pos].type != SY_THEN) {
        reportError("ȱ��then�ؼ���", tokens[pos]);
        return false;
    }
    pos++;

    // ����then����
    if (!parseStatement(tokens, pos)) {
        return false;
    }

    // ����else���ֵ���ת
    int skipElseJump = quadIndex;
    generateJump("", "", "", 0);  // ��ռλ���������

    // ��������Ϊ��ʱ����ת��ַ
    backPatch(falseJump, quadIndex);

    // ���else�ؼ���
    if (pos < tokens.size() && tokens[pos].type == SY_ELSE) {
        pos++;
        if (!parseStatement(tokens, pos)) {
            return false;
        }
    }

    // ��������else���ֵ���ת��ַ
    backPatch(skipElseJump, quadIndex);

    return true;
}

// ����while���
bool Parser::parseWhileStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "����while��俪ʼ" << std::endl;

    int startLabel = quadIndex;  // ѭ����ʼλ�ã����ں�����������ָ��
    pos++; // ����while

    // ��������
    if (pos < tokens.size() && tokens[pos].type == LPARENT) {
        pos++; // ����������
    }

    // �����������ʽ
    if (!parseBooleanExpression(tokens, pos)) {
        return false;
    }

    if (pos < tokens.size() && tokens[pos].type == RPARENT) {
        pos++; // ����������
    }

    // ����������ת
    int condJump = quadIndex;//��¼������תָ���λ��(���ں�������)
    int elseLabel = quadIndex + 2;  // Ԥ����ѭ�������λ��(ʵ�ʿ��ܲ�ͬ)
    generateJump("", "", "", elseLabel);

    // ���do�ؼ���
    if (pos >= tokens.size() || tokens[pos].type != SY_DO) {
        reportError("ȱ��do�ؼ���", tokens[pos]);
        return false;
    }
    pos++;

    // ����ѭ����
    if (!parseStatement(tokens, pos)) {
        return false;
    }

    // ����ѭ����ת�ؿ�ʼ
    generateJump("", "", "", startLabel);

    // ����ѭ��������λ��
    int endLabel = quadIndex;

    // ��֮ǰ���ɵ�ռλ��תָ���Ŀ���ַ���ΪendLabel
    backPatch(condJump, endLabel);

    return true;
}

// ������ֵ���
bool Parser::parseAssignmentStatement(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "������ֵ��俪ʼ" << std::endl;

    // ���沢�������ʶ��
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

    // �����Ҳ���ʽ
    if (!parseExpression(tokens, pos)) {
        return false;
    }

    // ���ɸ�ֵ��Ԫʽ
    generateQuadruple(":=", expressionResult, "", identifier);
    std::cout << "��ֵ���������" << std::endl;
    return true;
}

// �������ʽ���ݹ��½�������
bool Parser::parseExpression(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "�������ʽ��ʼ" << std::endl;

    if (!parseTerm(tokens, pos)) {
        return false;
    }
    std::string leftOperand = expressionResult;//�����������

    // ����ӷ�����
    while (pos < tokens.size() && tokens[pos].type == PLUS) {
        pos++; // �����Ӻ�
        //
        if (!parseTerm(tokens, pos)) {//�����Ҳ�����
            return false;
        }
        std::string rightOperand = expressionResult;//�����Ҳ��������

        // ������ʱ�����洢���
        std::string result = "T" + std::to_string(tempVarCounter++);
        generateQuadruple("+", leftOperand, rightOperand, result);//������Ԫʽ
        expressionResult = result;//���½��
        leftOperand = result;
    }

    // ����˷�����
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

    std::cout << "���ʽ�������" << std::endl;
    return true;
}

// ������
bool Parser::parseTerm(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "�����ʼ" << std::endl;

    if (!parseFactor(tokens, pos)) {
        return false;
    }

    // ����ӷ�����
    while (pos < tokens.size() && tokens[pos].type == PLUS) {
        pos++; // �����Ӻ�
        std::string leftOperand = expressionResult;//�����������
        if (!parseTerm(tokens, pos)) {//�����Ҳ�����
            return false;
        }
        std::string rightOperand = expressionResult;//�����Ҳ�����

        // ������ʱ�����洢���
        std::string result = "T" + std::to_string(tempVarCounter++);
        generateQuadruple("+", leftOperand, rightOperand, result);//������Ԫʽ
        expressionResult = result;//���½��
        leftOperand = result;//��֧����������
    }

    // ����˷�����
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

// ��������
bool Parser::parseFactor(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "�������ӿ�ʼ" << std::endl;

    if (pos >= tokens.size()) {//�ж��Ƿ񳬹�token���ȣ���+��������
        Token invalidToken;
        reportError("���ʽ�Ƿ�����", invalidToken);
        return false;
    }

    const Token& token = tokens[pos];
    switch (token.type) {
    case IDENT:  // ��ʶ��
    case INTCONST:  // ���ͳ���
        expressionResult = token.value;
        pos++;
        break;

    case LPARENT:  // ���ű��ʽ
        pos++; // ����������
        if (!parseExpression(tokens, pos)) {//�������ʽ
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

// ���������ŵĲ������ʽ
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

// �����������ʽ
bool Parser::parseBooleanExpression(const std::vector<Token>& tokens, size_t& pos) {
    std::cout << "�����������ʽ��ʼ" << std::endl;

    // �����������
    if (!parseExpression(tokens, pos)) {
        return false;
    }
    std::string leftOperand = expressionResult;//�����������

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
    std::string rightOperand = expressionResult;//�����Ҳ�����

    // ����������תָ��
    int nextQuad = quadIndex + 2;  // ���������ŵ���������ת
    generateJump(op, leftOperand, rightOperand, nextQuad);

    return true;
}

// ��ȡ���ɵ���Ԫʽ�б�
std::vector<std::string> Parser::getQuadruples() const {
    return quadruples;
}

// ��ȡ�����﷨��
Node* Parser::getAST() const {
    return ast;
}

// ��ȡ�ʷ���Ԫ��Ϣ�ַ���
std::string Parser::getTokenInfo(const Token& token) {
    std::stringstream ss;
    ss << "����: " << tokenTypeToString(token.type)//���ת���ַ���
        << ", ֵ: " << token.value
        << ", �к�: " << token.line;
    return ss.str();
}

// ���������Ϣ
void Parser::reportError(const std::string& message, const Token& token) {
    std::cerr << "�﷨����: " << message << " ";
    if (token.type != TokenType(-1)) {
        std::cerr << "�� " << getTokenInfo(token);
    }
    std::cerr << std::endl;
}

// ���ʷ���Ԫ����ת��Ϊ�ַ�����ʾ
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