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

    // 遍历语法树生成四元式
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

    // 递归处理子节点
    for (auto child : node->children) {
        processNode(child);
    }
}

void CodeGenerator::processExpression(Node* node) {
    if (node->children.size() == 2) {  // 二元运算
        std::string result = newTemp();
        std::string op = node->value;
        std::string arg1 = node->children[0]->value;
        std::string arg2 = node->children[1]->value;

        std::stringstream ss;
        ss << "(" << op << ", " << arg1 << ", " << arg2 << ", " << result << ")";
        quadruples.push_back(ss.str());

        node->value = result;  // 更新节点的值为临时变量
    }
}

void CodeGenerator::processStatement(Node* node) {
    if (node->value == "if") {
        std::string labelTrue = newLabel();
        std::string labelEnd = newLabel();

        // 生成条件判断的四元式
        std::stringstream ss;
        ss << "(jnz, " << node->children[0]->value << ", , " << labelTrue << ")";
        quadruples.push_back(ss.str());

        // 生成then部分的代码
        processNode(node->children[1]);

        // 生成跳转到结束的四元式
        ss.str("");
        ss << "(j, , , " << labelEnd << ")";
        quadruples.push_back(ss.str());

        // 生成else部分的标签
        ss.str("");
        ss << labelTrue << ":";
        quadruples.push_back(ss.str());

        // 生成else部分的代码
        if (node->children.size() > 2) {
            processNode(node->children[2]);
        }

        // 生成结束标签
        ss.str("");
        ss << labelEnd << ":";
        quadruples.push_back(ss.str());
    }
    else if (node->value == "while") {
        std::string labelStart = newLabel();
        std::string labelBody = newLabel();
        std::string labelEnd = newLabel();

        // 生成循环开始标签
        std::stringstream ss;
        ss << labelStart << ":";
        quadruples.push_back(ss.str());

        // 生成条件判断的四元式
        ss.str("");
        ss << "(jnz, " << node->children[0]->value << ", , " << labelBody << ")";
        quadruples.push_back(ss.str());

        // 生成跳转到结束的四元式
        ss.str("");
        ss << "(j, , , " << labelEnd << ")";
        quadruples.push_back(ss.str());

        // 生成循环体标签
        ss.str("");
        ss << labelBody << ":";
        quadruples.push_back(ss.str());

        // 生成循环体代码
        processNode(node->children[1]);

        // 生成跳回循环开始的四元式
        ss.str("");
        ss << "(j, , , " << labelStart << ")";
        quadruples.push_back(ss.str());

        // 生成结束标签
        ss.str("");
        ss << labelEnd << ":";
        quadruples.push_back(ss.str());
    }
}