#include "assembler.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <algorithm>

Assembler::Assembler() {}

bool Assembler::isNumber(const std::string& str) const {
    if (str.empty()) return false;
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

std::string Assembler::cleanVarName(const std::string& var) const {
    std::string cleaned;
    for (char c : var) {
        if (c != ',' && c != ')' && c != ' ') {
            cleaned += c;
        }
    }
    return cleaned;
}

void Assembler::generateDataSection() {
    asmCode.push_back("data segment");
    for (const auto& var : variables) {
        asmCode.push_back("    " + var + "           DW");
    }
    asmCode.push_back("data ends");
}

void Assembler::generateCodeSection() {
    asmCode.push_back("code segment");
    asmCode.push_back("main proc far");
    asmCode.push_back("    assume cs:code,ds:data");
    asmCode.push_back("");
    asmCode.push_back("start:");
    asmCode.push_back("    push ds");
    asmCode.push_back("    sub bx,bx");
    asmCode.push_back("push bx");
    asmCode.push_back("");
    asmCode.push_back("    mov bx,data");
    asmCode.push_back("    mov ds,bx");
}

void Assembler::processQuadruple(const std::vector<std::string>& quadruples) {
    // 首先收集所有变量
    variables = { "h", "k", "m", "n", "x", "y", "a", "b" };  // 预定义变量

    generateDataSection();
    generateCodeSection();

    // 处理每个四元式
    for (const auto& quad : quadruples) {
        std::stringstream ss(quad);
        int lineNum;
        char dummy;
        std::string op, arg1, arg2, result;
        ss >> lineNum;

        asmCode.push_back(std::to_string(lineNum) + ":");

        ss >> dummy >> op >> arg1;
        if (op[0] == 'j') {  // 跳转指令
            op = op.substr(1);  // 移除'j'前缀
            ss >> dummy >> arg2 >> dummy >> result;

            if (op.empty()) {
                asmCode.push_back("    jmp " + result);
            }
            else {
                arg1 = cleanVarName(arg1);
                arg2 = cleanVarName(arg2);
                asmCode.push_back("    mov AX," + arg1);
                asmCode.push_back("    cmp AX," + arg2);
                if (op == ">") asmCode.push_back("    jg  " + result);
                else if (op == ">=") asmCode.push_back("    jge " + result);
                else if (op == "=") asmCode.push_back("    je  " + result);
            }
        }
        else if (op == "+") {
            ss >> dummy >> arg2 >> dummy >> result;
            arg1 = cleanVarName(arg1);
            arg2 = cleanVarName(arg2);
            if (isNumber(arg2)) {
                asmCode.push_back("    mov AX," + arg1);
                asmCode.push_back("    add AX," + arg2 + "D");
            }
            else {
                asmCode.push_back("    mov AX," + arg1);
                asmCode.push_back("    add AX," + arg2);
            }
        }
        else if (op == "*") {
            ss >> dummy >> arg2 >> dummy >> result;
            arg1 = cleanVarName(arg1);
            arg2 = cleanVarName(arg2);
            asmCode.push_back("    mul " + arg2);
        }
        else if (op == ":=") {
            ss >> dummy >> arg2 >> dummy >> result;
            result = cleanVarName(result);
            asmCode.push_back("    mov BX,AX");
            asmCode.push_back("    mov " + result + ",BX");
        }
    }

    // 添加程序结束代码
    asmCode.push_back("    ret");
}

void Assembler::writeToFile(const std::string& filename) {
    std::ofstream outFile(filename);
    if (!outFile) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

    for (const auto& line : asmCode) {
        outFile << line << std::endl;
    }
    outFile.close();
}

void Assembler::printAssembly() const {
    for (const auto& line : asmCode) {
        std::cout << line << std::endl;
    }
}