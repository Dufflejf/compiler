#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <unordered_map>

// 四元式数据结构
struct Quadruple {
    int label;        // 标签/行号
    std::string op;   // 操作符
    std::string arg1; // 参数1
    std::string arg2; // 参数2
    std::string result; // 结果
};

// 判断是否为临时变量（T开头后跟数字）
bool is_temp(const std::string& s) {
    return !s.empty() && s[0] == 'T' && std::all_of(s.begin() + 1, s.end(), ::isdigit);
}

// 判断是否为数字常量
bool is_number(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

// 从文件解析四元式
std::vector<Quadruple> parse_quads(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<Quadruple> quads;
    std::string line;

    while (std::getline(file, line)) {
        // 跳过空行
        if (line.empty()) continue;

        // 提取标签号（行号）
        size_t start_pos = line.find('(');
        if (start_pos == std::string::npos) continue;

        std::string label_str = line.substr(0, line.find(' '));
        int label = std::stoi(label_str);

        // 提取括号内的内容
        std::string content = line.substr(start_pos + 1);
        content = content.substr(0, content.find(')'));

        // 按逗号分割各部分
        std::vector<std::string> parts;
        std::stringstream ss(content);
        std::string part;
        while (std::getline(ss, part, ',')) {
            // 去除前后空格
            part.erase(0, part.find_first_not_of(' '));
            part.erase(part.find_last_not_of(' ') + 1);
            parts.push_back(part);
        }

        // 确保有4个部分（不足补空）
        while (parts.size() < 4) parts.push_back("");

        // 构建四元式对象并加入列表
        Quadruple quad{ label, parts[0], parts[1], parts[2], parts[3] };
        quads.push_back(quad);
    }
    return quads;
}

// 收集所有变量（非临时变量和非数字）
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

// 生成汇编代码
void generate_assembly(const std::vector<Quadruple>& quads, const std::set<std::string>& vars, const std::string& output_filename) {
    std::ofstream asm_file(output_filename);
    std::cout << "生成汇编代码到: " << output_filename << std::endl;

    /******************** 汇编文件头 ********************/
    asm_file << ";************************************\n";
    asm_file << ";*  pas.asm                          *\n";
    asm_file << ";*  由四元式文件生成的汇编文件       *\n";
    asm_file << ";************************************\n\n";
    std::cout << ";************************************\n";
    std::cout << ";*  pas.asm                          *\n";
    std::cout << ";*  由四元式文件生成的汇编文件       *\n";
    std::cout << ";************************************\n\n";

    /******************** 数据段定义 ********************/
    asm_file << "data segment   \n";
    std::cout << "data segment   \n";
    // 为每个变量声明存储空间
    for (const auto& var : vars) {
        asm_file << "    " << var << "           DW ?\n";  // DW ? 表示分配一个字空间
        std::cout << "    " << var << "           DW ?\n";
    }
    asm_file << "data ends      \n\n";
    std::cout << "data ends      \n\n";

    /******************** 代码段定义 ********************/
    asm_file << "code segment    \n";
    asm_file << "main proc far   \n";
    asm_file << "    assume cs:code,ds:data\n\n";  // 设置段寄存器关联
    asm_file << "start:\n";
    // 标准程序初始化代码
    asm_file << "    push ds\n";      // 保存DS寄存器
    asm_file << "    sub bx,bx\n";    // BX清零
    asm_file << "    push bx\n";      // 压入返回地址
    asm_file << "    mov bx,data\n";  // 加载数据段地址
    asm_file << "    mov ds,bx\n";    // 设置DS寄存器

    // 控制台输出相同内容
    std::cout << "code segment    \n";
    std::cout << "main proc far   \n";
    std::cout << "    assume cs:code,ds:data\n\n";
    std::cout << "start:\n";
    std::cout << "    push ds\n";
    std::cout << "    sub bx,bx\n";
    std::cout << "    push bx\n";
    std::cout << "    mov bx,data\n";
    std::cout << "    mov ds,bx\n";

    // 临时变量到寄存器映射表
    std::unordered_map<std::string, std::string> temp_reg_map;

    /******************** 四元式转换 ********************/
    for (const auto& quad : quads) {
        // 输出标签作为汇编代码标号
        asm_file << quad.label << ":\n";
        std::cout << quad.label << ":\n";

        // 条件跳转指令处理（j>, j>=, j=）
        if (quad.op == "j>" || quad.op == "j>=" || quad.op == "j="||quad.op == "j<" || quad.op == "j<=" || quad.op == "j<>") {
            std::string arg1 = quad.arg1;
            std::string arg2 = quad.arg2;
            std::string jmp_target = quad.result;

            // 将第一个参数移动到AX寄存器
            asm_file << "    mov AX, " << arg1 << "\n";
            std::cout << "    mov AX, " << arg1 << "\n";

            // 比较指令
            asm_file << "    cmp AX, " << arg2 << "\n";
            std::cout << "    cmp AX, " << arg2 << "\n";

            // 根据操作符生成不同的条件跳转指令
            if (quad.op == "j>") {
                asm_file << "    jg  " << jmp_target << "\n";  // 大于跳转
                std::cout << "    jg  " << jmp_target << "\n";
            }
            else if (quad.op == "j>=") {
                asm_file << "    jge " << jmp_target << "\n";  // 大于等于跳转
                std::cout << "    jge " << jmp_target << "\n";
            }
            else if (quad.op == "j=") {
                asm_file << "    je  " << jmp_target << "\n";  // 等于跳转
                std::cout << "    je  " << jmp_target << "\n";
            }
            else if (quad.op == "j<") {
                asm_file << "    jl " << jmp_target << "\n";  // 小于跳转
                std::cout << "    jl " << jmp_target << "\n";
            }
            else if (quad.op == "j<=") {
                asm_file << "    jle  " << jmp_target << "\n";  // 小于等于跳转
                std::cout << "    jle  " << jmp_target << "\n";
            }
            else if (quad.op == "j<>") {
                asm_file << "    jne  " << jmp_target << "\n";  // 不等于跳转
                std::cout << "    jne  " << jmp_target << "\n";
            }
        }
        // 无条件跳转指令处理
        else if (quad.op == "j") {
            asm_file << "    jmp " << quad.result << "\n";
            std::cout << "    jmp " << quad.result << "\n";
        }
        // 赋值指令处理
        else if (quad.op == ":=") {
            // 如果源是临时变量，使用AX寄存器（之前操作结果）
            if (is_temp(quad.arg1)) {
                asm_file << "    mov " << quad.result << ", AX\n";
                std::cout << "    mov " << quad.result << ", AX\n";
            }
            else {
                // 直接赋值
                asm_file << "    mov AX, " << quad.arg1 << "\n";
                asm_file << "    mov " << quad.result << ", AX\n";
                std::cout << "    mov AX, " << quad.arg1 << "\n";
                std::cout << "    mov " << quad.result << ", AX\n";
            }
        }
        // 加法指令处理
        else if (quad.op == "+") {
            // 如果第一个操作数是临时变量，它已经在AX中
            if (is_temp(quad.arg1)) {
                if (is_temp(quad.arg2)) {
                    // 两个操作数都是临时变量
                    asm_file << "    mov BX, " << quad.arg2 << "\n";  // 加载第二个临时变量到BX
                    asm_file << "    add AX, BX\n";                   // 执行加法
                    std::cout << "    mov BX, " << quad.arg2 << "\n";
                    std::cout << "    add AX, BX\n";
                }
                else {
                    // 第二个操作数是数字或变量
                    asm_file << "    add AX, " << quad.arg2 << "D\n";  // D表示立即数
                    std::cout << "    add AX, " << quad.arg2 << "D\n";
                }
            }
            else {
                // 加载第一个操作数到AX
                asm_file << "    mov AX, " << quad.arg1 << "\n";
                std::cout << "    mov AX, " << quad.arg1 << "\n";
                if (is_temp(quad.arg2)) {
                    // 两个操作数都是临时变量
                    asm_file << "    mov BX, " << quad.arg2 << "\n";  // 加载第二个临时变量到BX
                    asm_file << "    add AX, BX\n";                   // 执行加法
                    std::cout << "    mov BX, " << quad.arg2 << "\n";
                    std::cout << "    add AX, BX\n";
                }
                else {
                    asm_file << "    add AX, " << quad.arg2 << "D\n";
                    std::cout << "    add AX, " << quad.arg2 << "D\n";
                }
            }
            // 结果存储在AX中供后续使用
            temp_reg_map[quad.result] = "AX";
        }
        // 乘法指令处理（针对特定示例）
        else if (quad.op == "*") {
            asm_file << "    mul "<< quad.arg1<<"\n";  // 乘以x（假设x是乘数）
            std::cout << "    mul "<< quad.arg1<<"\n";
            // 结果在AX中
            temp_reg_map[quad.result] = "AX";
        }
    }

    /******************** 程序结束部分 ********************/
    asm_file << "117:\n";  // 程序结束标签
    asm_file << "    ret\n";  // 返回操作系统
    asm_file << "main endp\n";  // 过程结束
    asm_file << "code ends\n";  // 代码段结束
    asm_file << "    end start\n";  // 程序结束，入口点为start
    std::cout << "117:\n";
    std::cout << "    ret\n";
    std::cout << "main endp\n";
    std::cout << "code ends\n";
    std::cout << "    end start\n";
}