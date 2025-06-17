#pragma once
#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <string>
#include <vector>
#include <set>

struct Quadruple {
    int label;
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;
};

bool is_temp(const std::string& s);
bool is_number(const std::string& s);
std::vector<Quadruple> parse_quads(const std::string& filename);
std::set<std::string> collect_vars(const std::vector<Quadruple>& quads);
void generate_assembly(const std::vector<Quadruple>& quads, const std::set<std::string>& vars, const std::string& output_filename);

#endif // ASSEMBLER_H