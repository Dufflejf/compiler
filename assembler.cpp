#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <unordered_map>

// ��Ԫʽ���ݽṹ
struct Quadruple {
    int label;        // ��ǩ/�к�
    std::string op;   // ������
    std::string arg1; // ����1
    std::string arg2; // ����2
    std::string result; // ���
};

// �ж��Ƿ�Ϊ��ʱ������T��ͷ������֣�
bool is_temp(const std::string& s) {
    return !s.empty() && s[0] == 'T' && std::all_of(s.begin() + 1, s.end(), ::isdigit);
}

// �ж��Ƿ�Ϊ���ֳ���
bool is_number(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

// ���ļ�������Ԫʽ
std::vector<Quadruple> parse_quads(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<Quadruple> quads;
    std::string line;

    while (std::getline(file, line)) {
        // ��������
        if (line.empty()) continue;

        // ��ȡ��ǩ�ţ��кţ�
        size_t start_pos = line.find('(');
        if (start_pos == std::string::npos) continue;

        std::string label_str = line.substr(0, line.find(' '));
        int label = std::stoi(label_str);

        // ��ȡ�����ڵ�����
        std::string content = line.substr(start_pos + 1);
        content = content.substr(0, content.find(')'));

        // �����ŷָ������
        std::vector<std::string> parts;
        std::stringstream ss(content);
        std::string part;
        while (std::getline(ss, part, ',')) {
            // ȥ��ǰ��ո�
            part.erase(0, part.find_first_not_of(' '));
            part.erase(part.find_last_not_of(' ') + 1);
            parts.push_back(part);
        }

        // ȷ����4�����֣����㲹�գ�
        while (parts.size() < 4) parts.push_back("");

        // ������Ԫʽ���󲢼����б�
        Quadruple quad{ label, parts[0], parts[1], parts[2], parts[3] };
        quads.push_back(quad);
    }
    return quads;
}

// �ռ����б���������ʱ�����ͷ����֣�
std::set<std::string> collect_vars(const std::vector<Quadruple>& quads) {
    std::set<std::string> vars;
    for (const auto& quad : quads) {
        if (!quad.arg1.empty() && !is_temp(quad.arg1) && !is_number(quad.arg1) && quad.arg1 != " ")
            vars.insert(quad.arg1);
        if (!quad.arg2.empty() && !is_temp(quad.arg2) && !is_number(quad.arg2) && quad.arg2 != " ")
            vars.insert(quad.arg2);
        if (!quad.result.empty() && !is_temp(quad.result) && !is_number(quad.result) && quad.result != " ")
            vars.insert(quad.result);
    }
    return vars;
}

// ���ɻ�����
void generate_assembly(const std::vector<Quadruple>& quads, const std::set<std::string>& vars, const std::string& output_filename) {
    std::ofstream asm_file(output_filename);
    std::cout << "���ɻ����뵽: " << output_filename << std::endl;

    /******************** ����ļ�ͷ ********************/
    asm_file << ";************************************\n";
    asm_file << ";*  pas.asm                          *\n";
    asm_file << ";*  ����Ԫʽ�ļ����ɵĻ���ļ�       *\n";
    asm_file << ";************************************\n\n";
    std::cout << ";************************************\n";
    std::cout << ";*  pas.asm                          *\n";
    std::cout << ";*  ����Ԫʽ�ļ����ɵĻ���ļ�       *\n";
    std::cout << ";************************************\n\n";

    /******************** ���ݶζ��� ********************/
    asm_file << "data segment   \n";
    std::cout << "data segment   \n";
    // Ϊÿ�����������洢�ռ�
    for (const auto& var : vars) {
        asm_file << "    " << var << "           DW ?\n";  // DW ? ��ʾ����һ���ֿռ�
        std::cout << "    " << var << "           DW ?\n";
    }
    asm_file << "data ends      \n\n";
    std::cout << "data ends      \n\n";

    /******************** ����ζ��� ********************/
    asm_file << "code segment    \n";
    asm_file << "main proc far   \n";
    asm_file << "    assume cs:code,ds:data\n\n";  // ���öμĴ�������
    asm_file << "start:\n";
    // ��׼�����ʼ������
    asm_file << "    push ds\n";      // ����DS�Ĵ���
    asm_file << "    sub bx,bx\n";    // BX����
    asm_file << "    push bx\n";      // ѹ�뷵�ص�ַ
    asm_file << "    mov bx,data\n";  // �������ݶε�ַ
    asm_file << "    mov ds,bx\n";    // ����DS�Ĵ���

    // ����̨�����ͬ����
    std::cout << "code segment    \n";
    std::cout << "main proc far   \n";
    std::cout << "    assume cs:code,ds:data\n\n";
    std::cout << "start:\n";
    std::cout << "    push ds\n";
    std::cout << "    sub bx,bx\n";
    std::cout << "    push bx\n";
    std::cout << "    mov bx,data\n";
    std::cout << "    mov ds,bx\n";

    // ��ʱ�������Ĵ���ӳ���
    std::unordered_map<std::string, std::string> temp_reg_map;

    /******************** ��Ԫʽת�� ********************/
    for (const auto& quad : quads) {
        // �����ǩ��Ϊ��������
        asm_file << quad.label << ":\n";
        std::cout << quad.label << ":\n";

        // ������תָ���j>, j>=, j=��
        if (quad.op == "j>" || quad.op == "j>=" || quad.op == "j="||quad.op == "j<" || quad.op == "j<=" || quad.op == "j<>") {
            std::string arg1 = quad.arg1;
            std::string arg2 = quad.arg2;
            std::string jmp_target = quad.result;

            // ����һ�������ƶ���AX�Ĵ���
            asm_file << "    mov AX, " << arg1 << "\n";
            std::cout << "    mov AX, " << arg1 << "\n";

            // �Ƚ�ָ��
            asm_file << "    cmp AX, " << arg2 << "\n";
            std::cout << "    cmp AX, " << arg2 << "\n";

            // ���ݲ��������ɲ�ͬ��������תָ��
            if (quad.op == "j>") {
                asm_file << "    jg  " << jmp_target << "\n";  // ������ת
                std::cout << "    jg  " << jmp_target << "\n";
            }
            else if (quad.op == "j>=") {
                asm_file << "    jge " << jmp_target << "\n";  // ���ڵ�����ת
                std::cout << "    jge " << jmp_target << "\n";
            }
            else if (quad.op == "j=") {
                asm_file << "    je  " << jmp_target << "\n";  // ������ת
                std::cout << "    je  " << jmp_target << "\n";
            }
            else if (quad.op == "j<") {
                asm_file << "    jl " << jmp_target << "\n";  // С����ת
                std::cout << "    jl " << jmp_target << "\n";
            }
            else if (quad.op == "j<=") {
                asm_file << "    jle  " << jmp_target << "\n";  // С�ڵ�����ת
                std::cout << "    jle  " << jmp_target << "\n";
            }
            else if (quad.op == "j<>") {
                asm_file << "    jne  " << jmp_target << "\n";  // ��������ת
                std::cout << "    jne  " << jmp_target << "\n";
            }
        }
        // ��������תָ���
        else if (quad.op == "j") {
            asm_file << "    jmp " << quad.result << "\n";
            std::cout << "    jmp " << quad.result << "\n";
        }
        // ��ֵָ���
        else if (quad.op == ":=") {
            // ���Դ����ʱ������ʹ��AX�Ĵ�����֮ǰ���������
            if (is_temp(quad.arg1)) {
                asm_file << "    mov " << quad.result << ", AX\n";
                std::cout << "    mov " << quad.result << ", AX\n";
            }
            else {
                // ֱ�Ӹ�ֵ
                asm_file << "    mov AX, " << quad.arg1 << "\n";
                asm_file << "    mov " << quad.result << ", AX\n";
                std::cout << "    mov AX, " << quad.arg1 << "\n";
                std::cout << "    mov " << quad.result << ", AX\n";
            }
        }
        // �ӷ�ָ���
        else if (quad.op == "+") {
            // �����һ������������ʱ���������Ѿ���AX��
            if (is_temp(quad.arg1)) {
                if (is_temp(quad.arg2)) {
                    // ����������������ʱ����
                    asm_file << "    mov BX, " << quad.arg2 << "\n";  // ���صڶ�����ʱ������BX
                    asm_file << "    add AX, BX\n";                   // ִ�мӷ�
                    std::cout << "    mov BX, " << quad.arg2 << "\n";
                    std::cout << "    add AX, BX\n";
                }
                else {
                    // �ڶ��������������ֻ����
                    asm_file << "    add AX, " << quad.arg2 << "D\n";  // D��ʾ������
                    std::cout << "    add AX, " << quad.arg2 << "D\n";
                }
            }
            else {
                // ���ص�һ����������AX
                asm_file << "    mov AX, " << quad.arg1 << "\n";
                std::cout << "    mov AX, " << quad.arg1 << "\n";
                if (is_temp(quad.arg2)) {
                    // ����������������ʱ����
                    asm_file << "    mov BX, " << quad.arg2 << "\n";  // ���صڶ�����ʱ������BX
                    asm_file << "    add AX, BX\n";                   // ִ�мӷ�
                    std::cout << "    mov BX, " << quad.arg2 << "\n";
                    std::cout << "    add AX, BX\n";
                }
                else {
                    asm_file << "    add AX, " << quad.arg2 << "D\n";
                    std::cout << "    add AX, " << quad.arg2 << "D\n";
                }
            }
            // ����洢��AX�й�����ʹ��
            temp_reg_map[quad.result] = "AX";
        }
        // �˷�ָ�������ض�ʾ����
        else if (quad.op == "*") {
            asm_file << "    mul "<< quad.arg1<<"\n";  // ����x������x�ǳ�����
            std::cout << "    mul "<< quad.arg1<<"\n";
            // �����AX��
            temp_reg_map[quad.result] = "AX";
        }
    }

    /******************** ����������� ********************/
    asm_file << "117:\n";  // ���������ǩ
    asm_file << "    ret\n";  // ���ز���ϵͳ
    asm_file << "main endp\n";  // ���̽���
    asm_file << "code ends\n";  // ����ν���
    asm_file << "    end start\n";  // �����������ڵ�Ϊstart
    std::cout << "117:\n";
    std::cout << "    ret\n";
    std::cout << "main endp\n";
    std::cout << "code ends\n";
    std::cout << "    end start\n";
}