#pragma once
#include "node.h"
#include <vector>
#include <string>

class CodeGenerator {
public:
    CodeGenerator();

    // ������Ԫʽ
    std::vector<std::string> generateQuadruples(Node* ast);

    // ���ɻ����루ѡ�����֣�
    std::vector<std::string> generateAssembly(const std::vector<std::string>& quadruples);

private:
    int labelCounter;
    int tempVarCounter;
    std::vector<std::string> quadruples;

    std::string newTemp();
    std::string newLabel();
    void processNode(Node* node);
    void processExpression(Node* node);
    void processStatement(Node* node);
};
