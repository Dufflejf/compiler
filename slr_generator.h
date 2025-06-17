#pragma once
#include "production.h"
#include "lr0_item.h"
#include <string>
#include <vector>
#include <map>
#include <set>

using ActionTable = std::map<int, std::map<std::string, std::string>>;
using GotoTable = std::map<int, std::map<std::string, std::string>>;

class SLRGenerator {
public:
    SLRGenerator();

    void generateArithmeticTable();
    void generateBooleanTable();
    void generateStatementTable();

private:
    std::vector<Production> productions;
    std::map<std::string, std::set<std::string>> first;
    std::map<std::string, std::set<std::string>> follow;
    std::vector<State> states;

    void initArithmeticGrammar();
    void initBooleanGrammar();
    void initStatementGrammar();
    void computeFirstSets();
    void computeFollowSets();
    void constructLR0Items();
    void generateParsingTable();
    bool isTerminal(const std::string& symbol);
    void closure(std::set<LR0Item>& items);
    std::set<LR0Item> computeGoto(const std::set<LR0Item>& items, const std::string& symbol);
    void printParsingTable(const ActionTable& actionTable, const GotoTable& gotoTable);
};