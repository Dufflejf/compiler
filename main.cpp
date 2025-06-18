#include "lexer.h"
#include "parser.h"
#include "slr_generator.h"
#include"assembler.h"
#include <iostream>
#include <fstream>
#include <string>

// ��ȡԴ�ļ�����
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("�޷���Դ�ļ���" + filename);
    }

    std::string content((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());
    file.close();
    return content;
}

// ������Ԫʽ��.med�ļ�
void saveMedFile(const std::vector<std::string>& quadruples, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("�޷�����.med�ļ���" + filename);
    }

    for (const auto& quad : quadruples) {
        file << quad << std::endl;
    }
    file.close();
}

int main() {
    try {
        // 1. ����SLR������
        SLRGenerator slrGen;
        std::cout << "��������SLR������..." << std::endl;
        slrGen.generateArithmeticTable();//�����������ʽSLR������
        slrGen.generateBooleanTable();//���ɲ������ʽSLR������
        slrGen.generateStatementTable();//���ɹ������SLR������

        // 2. ��ȡԴ�ļ�
        std::string sourceCode = readFile("pas.dat");
        std::cout << "\n��ȡ����Դ����\n" << sourceCode << std::endl;

        // 3. �ʷ�����
        Lexer lexer;
        std::vector<Token> tokens = lexer.tokenize(sourceCode);

        // ��ӡ�ʷ��������
        std::cout << "\n�ʷ����������" << std::endl;
        for (const auto& token : tokens) {
            std::cout << "Token: " << token.value
                << " (Type: " << token.type
                << ", Line: " << token.line << ")" << std::endl;
        }

        // 4. �﷨�������м��������
        Parser parser;
        if (parser.parse(tokens)) {
            std::cout << "�﷨�����ɹ���" << std::endl;

            // 5. ������Ԫʽ��.med�ļ�
            std::vector<std::string> quadruples = parser.getQuadruples();
            std::cout << "\n���ɵ���Ԫʽ��" << std::endl;
            for (const auto& quad : quadruples) {
                std::cout << quad << std::endl;
            }

            saveMedFile(quadruples, "pas.med");
            std::cout << "��Ԫʽ�ѱ��浽pas.med" << std::endl;
        }
        else {
            std::cout << "�﷨����ʧ�ܣ��������������﷨�Ƿ���ȷ��" << std::endl;
        }
        //5.������Է���
        std::vector<Quadruple> quads = parse_quads("pas.med");
        std::set<std::string> vars = collect_vars(quads);
        generate_assembly(quads, vars, "pas.asm");
    }
    catch (const std::exception& e) {
        std::cerr << "����" << e.what() << std::endl;
        return 1;
    }

    return 0;
}