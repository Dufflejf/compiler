#include "lexer.h"
#include "parser.h"
#include "slr_generator.h"
#include"assembler.h"
#include <iostream>
#include <fstream>
#include <string>

// 读取源文件内容
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开源文件：" + filename);
    }

    std::string content((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();
    return content;
}

// 保存四元式到.med文件
void saveMedFile(const std::vector<std::string>& quadruples, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("无法创建.med文件：" + filename);
    }

    for (const auto& quad : quadruples) {
        file << quad << std::endl;
    }
    file.close();
}

int main() {
    try {
        // 1. 生成SLR分析表
        SLRGenerator slrGen;
        std::cout << "正在生成SLR分析表..." << std::endl;
        slrGen.generateArithmeticTable();//生成算术表达式SLR分析表
        slrGen.generateBooleanTable();//生成布尔表达式SLR分析表
        slrGen.generateStatementTable();//生成过程语句SLR分析表

        // 2. 读取源文件
        std::string sourceCode = readFile("pas.dat");
        std::cout << "\n读取到的源程序：\n" << sourceCode << std::endl;

        // 3. 词法分析
        Lexer lexer;
        std::vector<Token> tokens = lexer.tokenize(sourceCode);

        // 打印词法分析结果
        std::cout << "\n词法分析结果：" << std::endl;
        for (const auto& token : tokens) {
            std::cout << "Token: " << token.value
                << " (Type: " << token.type
                << ", Line: " << token.line << ")" << std::endl;
        }

        // 4. 语法分析和中间代码生成
        Parser parser;
        if (parser.parse(tokens)) {
            std::cout << "语法分析成功！" << std::endl;

            // 5. 保存四元式到.med文件
            std::vector<std::string> quadruples = parser.getQuadruples();
            std::cout << "\n生成的四元式：" << std::endl;
            for (const auto& quad : quadruples) {
                std::cout << quad << std::endl;
            }

            saveMedFile(quadruples, "pas.med");
            std::cout << "四元式已保存到pas.med" << std::endl;
        }
        else {
            std::cout << "语法分析失败！请检查输入程序的语法是否正确。" << std::endl;
        }
        //5.汇编语言翻译
        std::vector<Quadruple> quads = parse_quads("pas.med");
        std::set<std::string> vars = collect_vars(quads);
        generate_assembly(quads, vars, "pas.asm");
    }
    catch (const std::exception& e) {
        std::cerr << "错误：" << e.what() << std::endl;
        return 1;
    }

    return 0;
}