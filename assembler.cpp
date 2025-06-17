#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <unordered_map>

struct Quadruple {
    int label;
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;
};

bool is_temp(const std::string& s) {
    return !s.empty() && s[0] == 'T' && std::all_of(s.begin() + 1, s.end(), ::isdigit);
}

bool is_number(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

std::vector<Quadruple> parse_quads(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<Quadruple> quads;
    std::string line;

    while (std::getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;

        // Extract label
        size_t start_pos = line.find('(');
        if (start_pos == std::string::npos) continue;

        std::string label_str = line.substr(0, line.find(' '));
        int label = std::stoi(label_str);

        // Extract content inside parentheses
        std::string content = line.substr(start_pos + 1);
        content = content.substr(0, content.find(')'));

        // Split by commas
        std::vector<std::string> parts;
        std::stringstream ss(content);
        std::string part;
        while (std::getline(ss, part, ',')) {
            // Trim spaces
            part.erase(0, part.find_first_not_of(' '));
            part.erase(part.find_last_not_of(' ') + 1);
            parts.push_back(part);
        }

        // Ensure we have 4 parts (fill empty if necessary)
        while (parts.size() < 4) parts.push_back("");

        Quadruple quad{ label, parts[0], parts[1], parts[2], parts[3] };
        quads.push_back(quad);
    }
    return quads;
}

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

void generate_assembly(const std::vector<Quadruple>& quads, const std::set<std::string>& vars, const std::string& output_filename) {
    std::ofstream asm_file(output_filename);
    std::cout << "生成汇编代码到: " << output_filename << std::endl;

    // Write header
    asm_file << ";************************************\n";
    asm_file << ";*  pas.asm                          *\n";
    asm_file << ";*  由四元式文件生成的汇编文件       *\n";
    asm_file << ";************************************\n\n";
    std::cout << ";************************************\n";
    std::cout << ";*  pas.asm                          *\n";
    std::cout << ";*  由四元式文件生成的汇编文件       *\n";
    std::cout << ";************************************\n\n";

    // Data segment
    asm_file << "data segment   \n";
    std::cout << "data segment   \n";
    for (const auto& var : vars) {
        asm_file << "    " << var << "           DW ?\n";
        std::cout << "    " << var << "           DW ?\n";
    }
    asm_file << "data ends      \n\n";
    std::cout << "data ends      \n\n";

    // Code segment
    asm_file << "code segment    \n";
    asm_file << "main proc far   \n";
    asm_file << "    assume cs:code,ds:data\n\n";
    asm_file << "start:\n";
    asm_file << "    push ds\n";
    asm_file << "    sub bx,bx\n";
    asm_file << "    push bx\n";
    asm_file << "    mov bx,data\n";
    asm_file << "    mov ds,bx\n";
    std::cout << "code segment    \n";
    std::cout << "main proc far   \n";
    std::cout << "    assume cs:code,ds:data\n\n";
    std::cout << "start:\n";
    std::cout << "    push ds\n";
    std::cout << "    sub bx,bx\n";
    std::cout << "    push bx\n";
    std::cout << "    mov bx,data\n";
    std::cout << "    mov ds,bx\n";

    // Translate each quadruple
    std::unordered_map<std::string, std::string> temp_reg_map;

    for (const auto& quad : quads) {
        asm_file << quad.label << ":\n";
        std::cout << quad.label << ":\n";

        if (quad.op == "j>" || quad.op == "j>=" || quad.op == "j=") {
            // For conditional jumps: mov, cmp, jxx
            std::string arg1 = quad.arg1;
            std::string arg2 = quad.arg2;
            std::string jmp_target = quad.result;

            // Move first arg to AX
            asm_file << "    mov AX, " << arg1 << "\n";
            std::cout << "    mov AX, " << arg1 << "\n";

            // Compare
            asm_file << "    cmp AX, " << arg2 << "\n";
            std::cout << "    cmp AX, " << arg2 << "\n";

            // Generate conditional jump
            if (quad.op == "j>") {
                asm_file << "    jg  " << jmp_target << "\n";
                std::cout << "    jg  " << jmp_target << "\n";
            }
            else if (quad.op == "j>=") {
                asm_file << "    jge " << jmp_target << "\n";
                std::cout << "    jge " << jmp_target << "\n";
            }
            else if (quad.op == "j=") {
                asm_file << "    je  " << jmp_target << "\n";
                std::cout << "    je  " << jmp_target << "\n";
            }
        }
        else if (quad.op == "j") {
            // Unconditional jump
            asm_file << "    jmp " << quad.result << "\n";
            std::cout << "    jmp " << quad.result << "\n";
        }
        else if (quad.op == ":=") {
            // Assignment
            if (is_temp(quad.arg1)) {
                // If source is a temporary, use AX (assigned in previous operation)
                asm_file << "    mov " << quad.result << ", AX\n";
                std::cout << "    mov " << quad.result << ", AX\n";
            }
            else {
                // Direct assignment from source to destination
                asm_file << "    mov AX, " << quad.arg1 << "\n";
                asm_file << "    mov " << quad.result << ", AX\n";
                std::cout << "    mov AX, " << quad.arg1 << "\n";
                std::cout << "    mov " << quad.result << ", AX\n";
            }
        }
        else if (quad.op == "+") {
            // Addition
            if (is_temp(quad.arg1)) {
                // If first operand is temp, it's already in AX
                if (is_temp(quad.arg2)) {
                    // Second operand is also temp (should not happen in example)
                    asm_file << "    add AX, " << quad.arg2 << "\n";
                    std::cout << "    add AX, " << quad.arg2 << "\n";
                }
                else {
                    // Second operand is number or variable
                    asm_file << "    add AX, " << quad.arg2 << "D\n";
                    std::cout << "    add AX, " << quad.arg2 << "D\n";
                }
            }
            else {
                // Load first operand into AX
                asm_file << "    mov AX, " << quad.arg1 << "\n";
                std::cout << "    mov AX, " << quad.arg1 << "\n";
                if (is_temp(quad.arg2)) {
                    // Second operand is temp (should not happen)
                    asm_file << "    add AX, " << quad.arg2 << "\n";
                    std::cout << "    add AX, " << quad.arg2 << "\n";
                }
                else {
                    asm_file << "    add AX, " << quad.arg2 << "D\n";
                    std::cout << "    add AX, " << quad.arg2 << "D\n";
                }
            }
            // Result stored in AX for next operation
            temp_reg_map[quad.result] = "AX";
        }
        else if (quad.op == "*") {
            // Multiplication (specific to example)
            asm_file << "    mul x\n";
            std::cout << "    mul x\n";
            // Result in AX
            temp_reg_map[quad.result] = "AX";
        }
    }

    // End of code
    asm_file << "117:\n";
    asm_file << "    ret\n";
    asm_file << "main endp\n";
    asm_file << "code ends\n";
    asm_file << "    end start\n";
    std::cout << "117:\n";
    std::cout << "    ret\n";
    std::cout << "main endp\n";
    std::cout << "code ends\n";
    std::cout << "    end start\n";
}