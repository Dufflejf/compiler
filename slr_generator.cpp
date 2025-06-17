#include "slr_generator.h"
#include <iostream>
#include <queue>
#include <algorithm>
#include <sstream>

SLRGenerator::SLRGenerator() {
    initArithmeticGrammar();
    initBooleanGrammar();
    initStatementGrammar();
}

void SLRGenerator::initArithmeticGrammar() {
    // 算术表达式文法
    // E → E+E | E*E | (E) | i
    productions.clear();
    productions.push_back(Production("S'", { "E" }));
    productions.push_back(Production("E", { "E", "+", "E" }));
    productions.push_back(Production("E", { "E", "*", "E" }));
    productions.push_back(Production("E", { "(", "E", ")" }));
    productions.push_back(Production("E", { "i" }));
}

void SLRGenerator::initBooleanGrammar() {
    // 布尔表达式文法
    productions.clear();
    productions.push_back(Production("S'", { "B" }));
    productions.push_back(Production("B", { "i" }));
    productions.push_back(Production("B", { "i", "rop", "i" }));
    productions.push_back(Production("B", { "(", "B", ")" }));
    productions.push_back(Production("B", { "not", "B" }));
    productions.push_back(Production("A", { "B", "and" }));
    productions.push_back(Production("B", { "A", "B" }));
    productions.push_back(Production("O", { "B", "or" }));
    productions.push_back(Production("B", { "O", "B" }));
}

void SLRGenerator::initStatementGrammar() {
    // 程序语句文法
    productions.clear();
    productions.push_back(Production("S'", { "S" }));
    productions.push_back(Production("S", { "if", "e", "then", "S", "else", "S" }));
    productions.push_back(Production("S", { "while", "e", "do", "S" }));
    productions.push_back(Production("S", { "begin", "L", "end" }));
    productions.push_back(Production("S", { "a" }));
    productions.push_back(Production("L", { "S" }));
    productions.push_back(Production("L", { "S", ";", "L" }));
}

bool SLRGenerator::isTerminal(const std::string& symbol) {
    static std::set<std::string> nonTerminals = {
        "S'", "S", "E", "B", "A", "O", "L"
    };
    return nonTerminals.find(symbol) == nonTerminals.end();
}

void SLRGenerator::computeFirstSets() {
    bool changed;
    do {
        changed = false;
        for (const auto& prod : productions) {
            std::set<std::string>& firstSet = first[prod.left];
            size_t oldSize = firstSet.size();

            if (prod.right.empty()) {
                firstSet.insert("ε");
                continue;
            }

            // 处理右部第一个符号
            if (isTerminal(prod.right[0])) {
                firstSet.insert(prod.right[0]);
            }
            else {
                const auto& rightFirst = first[prod.right[0]];
                firstSet.insert(rightFirst.begin(), rightFirst.end());
            }

            if (firstSet.size() > oldSize) {
                changed = true;
            }
        }
    } while (changed);
}

void SLRGenerator::computeFollowSets() {
    // 初始化，将#加入到开始符号的FOLLOW集中
    follow[productions[0].left].insert("#");

    bool changed;
    do {
        changed = false;
        for (const auto& prod : productions) {
            for (size_t i = 0; i < prod.right.size(); i++) {
                if (isTerminal(prod.right[i])) continue;

                std::set<std::string>& followSet = follow[prod.right[i]];
                size_t oldSize = followSet.size();

                // 如果不是最后一个符号
                if (i < prod.right.size() - 1) {
                    if (isTerminal(prod.right[i + 1])) {
                        followSet.insert(prod.right[i + 1]);
                    }
                    else {
                        const auto& nextFirst = first[prod.right[i + 1]];
                        followSet.insert(nextFirst.begin(), nextFirst.end());
                    }
                }
                // 如果是最后一个符号或者下一个符号的FIRST集包含ε
                else {
                    const auto& leftFollow = follow[prod.left];
                    followSet.insert(leftFollow.begin(), leftFollow.end());
                }

                if (followSet.size() > oldSize) {
                    changed = true;
                }
            }
        }
    } while (changed);
}

void SLRGenerator::closure(std::set<LR0Item>& items) {
    bool changed;
    do {
        changed = false;
        std::set<LR0Item> newItems = items;

        for (const auto& item : items) {
            // 如果点号后面是非终结符
            if (item.dotPos < item.prod.right.size() &&
                !isTerminal(item.prod.right[item.dotPos])) {

                std::string nextSymbol = item.prod.right[item.dotPos];

                // 添加所有以该非终结符为左部的产生式
                for (const auto& prod : productions) {
                    if (prod.left == nextSymbol) {
                        LR0Item newItem(prod, 0);
                        if (newItems.insert(newItem).second) {
                            changed = true;
                        }
                    }
                }
            }
        }

        items = std::move(newItems);
    } while (changed);
}

std::set<LR0Item> SLRGenerator::computeGoto(const std::set<LR0Item>& items,
    const std::string& symbol) {
    std::set<LR0Item> gotoSet;

    // 对当前项目集中的每个项目
    for (const auto& item : items) {
        // 如果点号没有到达末尾且点号后的符号等于给定符号
        if (item.dotPos < item.prod.right.size() &&
            item.prod.right[item.dotPos] == symbol) {

            // 创建新项目，将点号向后移动一位
            LR0Item newItem(item.prod, item.dotPos + 1);
            gotoSet.insert(newItem);
        }
    }

    // 对结果项目集求闭包
    closure(gotoSet);
    return gotoSet;
}

void SLRGenerator::constructLR0Items() {
    states.clear();
    std::map<std::set<LR0Item>, int> stateMap;

    // 创建初始项目集
    std::set<LR0Item> initialItems;
    initialItems.insert(LR0Item(productions[0], 0));
    closure(initialItems);

    // 创建初始状态
    states.push_back(State(initialItems, 0));
    stateMap[initialItems] = 0;

    // 使用队列进行广度优先搜索
    std::queue<int> queue;
    queue.push(0);

    while (!queue.empty()) {
        int currentState = queue.front();
        queue.pop();

        // 获取当前状态的所有符号
        std::set<std::string> symbols;
        for (const auto& item : states[currentState].items) {
            if (item.dotPos < item.prod.right.size()) {
                symbols.insert(item.prod.right[item.dotPos]);
            }
        }

        // 对每个符号计算GOTO集
        for (const auto& symbol : symbols) {
            std::set<LR0Item> gotoSet = computeGoto(states[currentState].items, symbol);

            if (!gotoSet.empty()) {
                // 检查是否是新状态
                if (stateMap.find(gotoSet) == stateMap.end()) {
                    int newStateNum = states.size();
                    states.push_back(State(gotoSet, newStateNum));
                    stateMap[gotoSet] = newStateNum;
                    queue.push(newStateNum);
                }

                // 添加转换
                states[currentState].transitions[symbol] = stateMap[gotoSet];
            }
        }
    }
}

void SLRGenerator::generateParsingTable() {
    // 计算FIRST和FOLLOW集
    computeFirstSets();
    computeFollowSets();

    // 构造项目集规范族
    constructLR0Items();

    // 生成分析表
    ActionTable actionTable;
    GotoTable gotoTable;

    for (const auto& state : states) {
        for (const auto& item : state.items) {
            // 如果点号在末尾
            if (item.dotPos == item.prod.right.size()) {
                // 规约
                if (item.prod.left == "S'") {
                    // 接受
                    actionTable[state.stateNum]["#"] = "acc";
                }
                else {
                    // 查找产生式编号
                    int prodIndex = 0;
                    for (size_t i = 0; i < productions.size(); i++) {
                        if (productions[i] == item.prod) {
                            prodIndex = i;
                            break;
                        }
                    }

                    // 对Follow集中的所有符号添加规约动作
                    for (const auto& symbol : follow[item.prod.left]) {
                        actionTable[state.stateNum][symbol] = "r" + std::to_string(prodIndex);
                    }
                }
            }
            else {
                // 移进
                std::string nextSymbol = item.prod.right[item.dotPos];
                if (state.transitions.count(nextSymbol) > 0) {
                    if (isTerminal(nextSymbol)) {
                        // 使用 at() 代替 []
                        int nextState = state.transitions.at(nextSymbol);
                        actionTable[state.stateNum][nextSymbol] =
                            "s" + std::to_string(nextState);
                    }
                    else {
                        // 使用 at() 代替 []
                        int nextState = state.transitions.at(nextSymbol);
                        gotoTable[state.stateNum][nextSymbol] =
                            std::to_string(nextState);
                    }
                }
            }
        }
    }

    // 打印分析表
    std::cout << "\nSLR分析表：" << std::endl;
    printParsingTable(actionTable, gotoTable);
}

void SLRGenerator::printParsingTable(const ActionTable& actionTable,
    const GotoTable& gotoTable) {
    // 获取所有终结符和非终结符
    std::set<std::string> terminals, nonTerminals;
    for (const auto& prod : productions) {
        if (!isTerminal(prod.left)) {
            nonTerminals.insert(prod.left);
        }
        for (const auto& symbol : prod.right) {
            if (isTerminal(symbol)) {
                terminals.insert(symbol);
            }
            else {
                nonTerminals.insert(symbol);
            }
        }
    }
    terminals.insert("#");

    // 打印表头
    std::cout << "状态\t";
    for (const auto& term : terminals) {
        std::cout << term << "\t";
    }
    for (const auto& nonTerm : nonTerminals) {
        if (nonTerm != "S'") {
            std::cout << nonTerm << "\t";
        }
    }
    std::cout << std::endl;

    // 打印每一行
    for (const auto& state : states) {
        std::cout << state.stateNum << "\t";

        // 打印ACTION部分
        for (const auto& term : terminals) {
            if (actionTable.count(state.stateNum) &&
                actionTable.at(state.stateNum).count(term)) {
                std::cout << actionTable.at(state.stateNum).at(term);
            }
            std::cout << "\t";
        }

        // 打印GOTO部分
        for (const auto& nonTerm : nonTerminals) {
            if (nonTerm != "S'" &&
                gotoTable.count(state.stateNum) &&
                gotoTable.at(state.stateNum).count(nonTerm)) {
                std::cout << gotoTable.at(state.stateNum).at(nonTerm);
            }
            std::cout << "\t";
        }
        std::cout << std::endl;
    }
}

void SLRGenerator::generateArithmeticTable() {
    std::cout << "生成算术表达式SLR分析表..." << std::endl;
    initArithmeticGrammar();
    generateParsingTable();
}

void SLRGenerator::generateBooleanTable() {
    std::cout << "生成布尔表达式SLR分析表..." << std::endl;
    initBooleanGrammar();
    generateParsingTable();
}

void SLRGenerator::generateStatementTable() {
    std::cout << "生成程序语句SLR分析表..." << std::endl;
    initStatementGrammar();
    generateParsingTable();
}