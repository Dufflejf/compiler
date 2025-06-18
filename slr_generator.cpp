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

// 计算所有非终结符的FIRST集合
void SLRGenerator::computeFirstSets() {
    bool changed;  // 标记是否有FIRST集合被更新
    do {
        changed = false;  // 每轮开始前重置标记
        // 遍历文法中的所有产生式
        for (const auto& prod : productions) {
            // 获取当前产生式左部非终结符的FIRST集合引用
            std::set<std::string>& firstSet = first[prod.left];
            size_t oldSize = firstSet.size();  // 记录当前FIRST集合大小
            // 处理空产生式情况（右部为空）
            if (prod.right.empty()) {
                firstSet.insert("ε");  // 加入空串ε
                continue;  // 跳过后续处理
            }
            // 处理产生式右部的第一个符号
            if (isTerminal(prod.right[0])) {
                // 如果是终结符，直接加入FIRST集合
                firstSet.insert(prod.right[0]);
            }
            else {
                // 如果是非终结符，将其FIRST集合内容加入当前FIRST集合
                const auto& rightFirst = first[prod.right[0]];
                firstSet.insert(rightFirst.begin(), rightFirst.end());
            }
            // 检查FIRST集合是否发生变化
            if (firstSet.size() > oldSize) {
                changed = true;  // 如果大小变化，标记需要继续迭代
            }
        }
    } while (changed);  // 当没有FIRST集合更新时停止循环
}

// 计算所有非终结符的FOLLOW集合
void SLRGenerator::computeFollowSets() {
    // 初始化，将结束符#加入到文法开始符号的FOLLOW集中
    follow[productions[0].left].insert("#");
    bool changed;  // 标记是否有FOLLOW集合被更新
    do {
        changed = false;  // 每轮开始前重置标记
        // 遍历文法中的所有产生式
        for (const auto& prod : productions) {
            // 遍历产生式右部的每个符号
            for (size_t i = 0; i < prod.right.size(); i++) {
                // 跳过终结符，只处理非终结符
                if (isTerminal(prod.right[i])) continue;
                // 获取当前非终结符的FOLLOW集合引用
                std::set<std::string>& followSet = follow[prod.right[i]];
                size_t oldSize = followSet.size();  // 记录当前FOLLOW集合大小
                // 情况1：当前符号不是产生式最后一个符号
                if (i < prod.right.size() - 1) {
                    // 情况1.1：下一个符号是终结符
                    if (isTerminal(prod.right[i + 1])) {
                        followSet.insert(prod.right[i + 1]);  // 直接加入FOLLOW集
                    }
                    // 情况1.2：下一个符号是非终结符
                    else {
                        // 将下一个非终结符的FIRST集加入当前FOLLOW集
                        const auto& nextFirst = first[prod.right[i + 1]];
                        followSet.insert(nextFirst.begin(), nextFirst.end());
                    }
                }
                // 情况2：当前符号是产生式最后一个符号
                // 或者下一个符号的FIRST集包含ε
                else {
                    // 将产生式左部非终结符的FOLLOW集加入当前FOLLOW集
                    const auto& leftFollow = follow[prod.left];
                    followSet.insert(leftFollow.begin(), leftFollow.end());
                }
                // 检查FOLLOW集合是否发生变化
                if (followSet.size() > oldSize) {
                    changed = true;  // 如果大小变化，标记需要继续迭代
                }
            }
        }
    } while (changed);  // 当没有FOLLOW集合更新时停止循环
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
                        if (newItems.insert(newItem).second) {//加入newItem
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

// 构造LR(0)自动机的所有项目集（状态）
void SLRGenerator::constructLR0Items() {
    states.clear();  // 清空现有状态集合
    std::map<std::set<LR0Item>, int> stateMap;  // 用于记录项目集到状态编号的映射
    // 1. 创建初始项目集（包含增广文法的第一个产生式）
    std::set<LR0Item> initialItems;
    initialItems.insert(LR0Item(productions[0], 0));  // 加入初始项目: S' -> .S
    closure(initialItems);  // 计算初始项目集的闭包
    // 2. 创建初始状态（状态0）
    states.push_back(State(initialItems, 0));  // 将初始项目集加入状态集合
    stateMap[initialItems] = 0;  // 记录项目集到状态的映射
    // 3. 使用队列进行广度优先搜索（BFS）构建所有状态
    std::queue<int> queue;  // 用于BFS的状态队列
    queue.push(0);  // 从初始状态开始处理
    while (!queue.empty()) {
        int currentState = queue.front();  // 获取当前处理的状态编号
        queue.pop();  // 从队列中移除
        // 4. 收集当前状态所有可能的转移符号（圆点后的符号）
        std::set<std::string> symbols;
        for (const auto& item : states[currentState].items) {
            if (item.dotPos < item.prod.right.size()) {  // 检查圆点是否在产生式末尾
                symbols.insert(item.prod.right[item.dotPos]);  // 添加圆点后的符号
            }
        }
        // 5. 对每个可能的转移符号计算GOTO集合，状态 I 通过符号 X 转移到的目标状态
        for (const auto& symbol : symbols) {
            // 计算GOTO(I, symbol)
            std::set<LR0Item> gotoSet = computeGoto(states[currentState].items, symbol);
            if (!gotoSet.empty()) {  // 如果GOTO集合不为空
                // 6. 检查是否是新状态
                if (stateMap.find(gotoSet) == stateMap.end()) {
                    // 创建新状态
                    int newStateNum = states.size();  // 新状态编号
                    states.push_back(State(gotoSet, newStateNum));  // 添加到状态集合
                    stateMap[gotoSet] = newStateNum;  // 更新项目集到状态的映射
                    queue.push(newStateNum);  // 将新状态加入处理队列
                }
                // 7. 添加状态转换（当前状态通过symbol转移到目标状态）
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
    initArithmeticGrammar();//初始化文法，下面相同
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