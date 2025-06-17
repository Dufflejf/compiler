#include "code_generator.h"
#include <sstream>

CodeGenerator::CodeGenerator() : labelCounter(0), tempVarCounter(0) {}

std::string CodeGenerator::newTemp() {
    return "T" + std::to_string(++tempVarCounter);
}

std::string CodeGenerator::newLabel() {
    return "L" + std::to_string(++labelCounter);
}

std::vector<std::string> CodeGenerator::generateQuadruples(Node* ast) {
    std::vector<std::string> quadruples;

    // �����﷨��������Ԫʽ
    processNode(ast);

    return quadruples;
}

void CodeGenerator::processNode(Node* node) {
    if (node == nullptr) return;

    if (node->type == "Expression") {
        processExpression(node);
    }
    else if (node->type == "Statement") {
        processStatement(node);
    }

    // �ݹ鴦���ӽڵ�
    for (auto child : node->children) {
        processNode(child);
    }
}

void CodeGenerator::processExpression(Node* node) {
    if (node->children.size() == 2) {  // ��Ԫ����
        std::string result = newTemp();
        std::string op = node->value;
        std::string arg1 = node->children[0]->value;
        std::string arg2 = node->children[1]->value;

        std::stringstream ss;
        ss << "(" << op << ", " << arg1 << ", " << arg2 << ", " << result << ")";
        quadruples.push_back(ss.str());

        node->value = result;  // ���½ڵ��ֵΪ��ʱ����
    }
}

void CodeGenerator::processStatement(Node* node) {
    if (node->value == "if") {
        std::string labelTrue = newLabel();
        std::string labelEnd = newLabel();

        // ���������жϵ���Ԫʽ
        std::stringstream ss;
        ss << "(jnz, " << node->children[0]->value << ", , " << labelTrue << ")";
        quadruples.push_back(ss.str());

        // ����then���ֵĴ���
        processNode(node->children[1]);

        // ������ת����������Ԫʽ
        ss.str("");
        ss << "(j, , , " << labelEnd << ")";
        quadruples.push_back(ss.str());

        // ����else���ֵı�ǩ
        ss.str("");
        ss << labelTrue << ":";
        quadruples.push_back(ss.str());

        // ����else���ֵĴ���
        if (node->children.size() > 2) {
            processNode(node->children[2]);
        }

        // ���ɽ�����ǩ
        ss.str("");
        ss << labelEnd << ":";
        quadruples.push_back(ss.str());
    }
    else if (node->value == "while") {
        std::string labelStart = newLabel();
        std::string labelBody = newLabel();
        std::string labelEnd = newLabel();

        // ����ѭ����ʼ��ǩ
        std::stringstream ss;
        ss << labelStart << ":";
        quadruples.push_back(ss.str());

        // ���������жϵ���Ԫʽ
        ss.str("");
        ss << "(jnz, " << node->children[0]->value << ", , " << labelBody << ")";
        quadruples.push_back(ss.str());

        // ������ת����������Ԫʽ
        ss.str("");
        ss << "(j, , , " << labelEnd << ")";
        quadruples.push_back(ss.str());

        // ����ѭ�����ǩ
        ss.str("");
        ss << labelBody << ":";
        quadruples.push_back(ss.str());

        // ����ѭ�������
        processNode(node->children[1]);

        // ��������ѭ����ʼ����Ԫʽ
        ss.str("");
        ss << "(j, , , " << labelStart << ")";
        quadruples.push_back(ss.str());

        // ���ɽ�����ǩ
        ss.str("");
        ss << labelEnd << ":";
        quadruples.push_back(ss.str());
    }
}