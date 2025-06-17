#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>

class Assembler {
public:
    Assembler();
    void processQuadruple(const std::vector<std::string>& quadruples);
    void writeToFile(const std::string& filename);
    void printAssembly() const;  // ��������ӡ������

private:
    std::vector<std::string> asmCode;
    std::set<std::string> variables;  // ʹ��set�洢������

    void generateDataSection();
    void generateCodeSection();
    void processArithmetic(const std::string& op, const std::string& arg1,
        const std::string& arg2, const std::string& result);
    void processJump(const std::string& op, const std::string& arg1,
        const std::string& arg2, const std::string& target);
    void processAssignment(const std::string& source, const std::string& target);
    bool isNumber(const std::string& str) const;
    std::string cleanVarName(const std::string& var) const;  // ���������������
};
