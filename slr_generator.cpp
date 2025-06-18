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
    // �������ʽ�ķ�
    // E �� E+E | E*E | (E) | i
    productions.clear();
    productions.push_back(Production("S'", { "E" }));
    productions.push_back(Production("E", { "E", "+", "E" }));
    productions.push_back(Production("E", { "E", "*", "E" }));
    productions.push_back(Production("E", { "(", "E", ")" }));
    productions.push_back(Production("E", { "i" }));
}

void SLRGenerator::initBooleanGrammar() {
    // �������ʽ�ķ�
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
    // ��������ķ�
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

// �������з��ս����FIRST����
void SLRGenerator::computeFirstSets() {
    bool changed;  // ����Ƿ���FIRST���ϱ�����
    do {
        changed = false;  // ÿ�ֿ�ʼǰ���ñ��
        // �����ķ��е����в���ʽ
        for (const auto& prod : productions) {
            // ��ȡ��ǰ����ʽ�󲿷��ս����FIRST��������
            std::set<std::string>& firstSet = first[prod.left];
            size_t oldSize = firstSet.size();  // ��¼��ǰFIRST���ϴ�С
            // ����ղ���ʽ������Ҳ�Ϊ�գ�
            if (prod.right.empty()) {
                firstSet.insert("��");  // ����մ���
                continue;  // ������������
            }
            // �������ʽ�Ҳ��ĵ�һ������
            if (isTerminal(prod.right[0])) {
                // ������ս����ֱ�Ӽ���FIRST����
                firstSet.insert(prod.right[0]);
            }
            else {
                // ����Ƿ��ս��������FIRST�������ݼ��뵱ǰFIRST����
                const auto& rightFirst = first[prod.right[0]];
                firstSet.insert(rightFirst.begin(), rightFirst.end());
            }
            // ���FIRST�����Ƿ����仯
            if (firstSet.size() > oldSize) {
                changed = true;  // �����С�仯�������Ҫ��������
            }
        }
    } while (changed);  // ��û��FIRST���ϸ���ʱֹͣѭ��
}

// �������з��ս����FOLLOW����
void SLRGenerator::computeFollowSets() {
    // ��ʼ������������#���뵽�ķ���ʼ���ŵ�FOLLOW����
    follow[productions[0].left].insert("#");
    bool changed;  // ����Ƿ���FOLLOW���ϱ�����
    do {
        changed = false;  // ÿ�ֿ�ʼǰ���ñ��
        // �����ķ��е����в���ʽ
        for (const auto& prod : productions) {
            // ��������ʽ�Ҳ���ÿ������
            for (size_t i = 0; i < prod.right.size(); i++) {
                // �����ս����ֻ������ս��
                if (isTerminal(prod.right[i])) continue;
                // ��ȡ��ǰ���ս����FOLLOW��������
                std::set<std::string>& followSet = follow[prod.right[i]];
                size_t oldSize = followSet.size();  // ��¼��ǰFOLLOW���ϴ�С
                // ���1����ǰ���Ų��ǲ���ʽ���һ������
                if (i < prod.right.size() - 1) {
                    // ���1.1����һ���������ս��
                    if (isTerminal(prod.right[i + 1])) {
                        followSet.insert(prod.right[i + 1]);  // ֱ�Ӽ���FOLLOW��
                    }
                    // ���1.2����һ�������Ƿ��ս��
                    else {
                        // ����һ�����ս����FIRST�����뵱ǰFOLLOW��
                        const auto& nextFirst = first[prod.right[i + 1]];
                        followSet.insert(nextFirst.begin(), nextFirst.end());
                    }
                }
                // ���2����ǰ�����ǲ���ʽ���һ������
                // ������һ�����ŵ�FIRST��������
                else {
                    // ������ʽ�󲿷��ս����FOLLOW�����뵱ǰFOLLOW��
                    const auto& leftFollow = follow[prod.left];
                    followSet.insert(leftFollow.begin(), leftFollow.end());
                }
                // ���FOLLOW�����Ƿ����仯
                if (followSet.size() > oldSize) {
                    changed = true;  // �����С�仯�������Ҫ��������
                }
            }
        }
    } while (changed);  // ��û��FOLLOW���ϸ���ʱֹͣѭ��
}

void SLRGenerator::closure(std::set<LR0Item>& items) {
    bool changed;
    do {
        changed = false;
        std::set<LR0Item> newItems = items;

        for (const auto& item : items) {
            // �����ź����Ƿ��ս��
            if (item.dotPos < item.prod.right.size() &&
                !isTerminal(item.prod.right[item.dotPos])) {

                std::string nextSymbol = item.prod.right[item.dotPos];

                // ��������Ը÷��ս��Ϊ�󲿵Ĳ���ʽ
                for (const auto& prod : productions) {
                    if (prod.left == nextSymbol) {
                        LR0Item newItem(prod, 0);
                        if (newItems.insert(newItem).second) {//����newItem
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

    // �Ե�ǰ��Ŀ���е�ÿ����Ŀ
    for (const auto& item : items) {
        // ������û�е���ĩβ�ҵ�ź�ķ��ŵ��ڸ�������
        if (item.dotPos < item.prod.right.size() &&
            item.prod.right[item.dotPos] == symbol) {

            // ��������Ŀ�����������ƶ�һλ
            LR0Item newItem(item.prod, item.dotPos + 1);
            gotoSet.insert(newItem);
        }
    }

    // �Խ����Ŀ����հ�
    closure(gotoSet);
    return gotoSet;
}

// ����LR(0)�Զ�����������Ŀ����״̬��
void SLRGenerator::constructLR0Items() {
    states.clear();  // �������״̬����
    std::map<std::set<LR0Item>, int> stateMap;  // ���ڼ�¼��Ŀ����״̬��ŵ�ӳ��
    // 1. ������ʼ��Ŀ�������������ķ��ĵ�һ������ʽ��
    std::set<LR0Item> initialItems;
    initialItems.insert(LR0Item(productions[0], 0));  // �����ʼ��Ŀ: S' -> .S
    closure(initialItems);  // �����ʼ��Ŀ���ıհ�
    // 2. ������ʼ״̬��״̬0��
    states.push_back(State(initialItems, 0));  // ����ʼ��Ŀ������״̬����
    stateMap[initialItems] = 0;  // ��¼��Ŀ����״̬��ӳ��
    // 3. ʹ�ö��н��й������������BFS����������״̬
    std::queue<int> queue;  // ����BFS��״̬����
    queue.push(0);  // �ӳ�ʼ״̬��ʼ����
    while (!queue.empty()) {
        int currentState = queue.front();  // ��ȡ��ǰ�����״̬���
        queue.pop();  // �Ӷ������Ƴ�
        // 4. �ռ���ǰ״̬���п��ܵ�ת�Ʒ��ţ�Բ���ķ��ţ�
        std::set<std::string> symbols;
        for (const auto& item : states[currentState].items) {
            if (item.dotPos < item.prod.right.size()) {  // ���Բ���Ƿ��ڲ���ʽĩβ
                symbols.insert(item.prod.right[item.dotPos]);  // ���Բ���ķ���
            }
        }
        // 5. ��ÿ�����ܵ�ת�Ʒ��ż���GOTO���ϣ�״̬ I ͨ������ X ת�Ƶ���Ŀ��״̬
        for (const auto& symbol : symbols) {
            // ����GOTO(I, symbol)
            std::set<LR0Item> gotoSet = computeGoto(states[currentState].items, symbol);
            if (!gotoSet.empty()) {  // ���GOTO���ϲ�Ϊ��
                // 6. ����Ƿ�����״̬
                if (stateMap.find(gotoSet) == stateMap.end()) {
                    // ������״̬
                    int newStateNum = states.size();  // ��״̬���
                    states.push_back(State(gotoSet, newStateNum));  // ��ӵ�״̬����
                    stateMap[gotoSet] = newStateNum;  // ������Ŀ����״̬��ӳ��
                    queue.push(newStateNum);  // ����״̬���봦�����
                }
                // 7. ���״̬ת������ǰ״̬ͨ��symbolת�Ƶ�Ŀ��״̬��
                states[currentState].transitions[symbol] = stateMap[gotoSet];
            }
        }
    }
}

void SLRGenerator::generateParsingTable() {
    // ����FIRST��FOLLOW��
    computeFirstSets();
    computeFollowSets();

    // ������Ŀ���淶��
    constructLR0Items();

    // ���ɷ�����
    ActionTable actionTable;
    GotoTable gotoTable;

    for (const auto& state : states) {
        for (const auto& item : state.items) {
            // ��������ĩβ
            if (item.dotPos == item.prod.right.size()) {
                // ��Լ
                if (item.prod.left == "S'") {
                    // ����
                    actionTable[state.stateNum]["#"] = "acc";
                }
                else {
                    // ���Ҳ���ʽ���
                    int prodIndex = 0;
                    for (size_t i = 0; i < productions.size(); i++) {
                        if (productions[i] == item.prod) {
                            prodIndex = i;
                            break;
                        }
                    }

                    // ��Follow���е����з�����ӹ�Լ����
                    for (const auto& symbol : follow[item.prod.left]) {
                        actionTable[state.stateNum][symbol] = "r" + std::to_string(prodIndex);
                    }
                }
            }
            else {
                // �ƽ�
                std::string nextSymbol = item.prod.right[item.dotPos];
                if (state.transitions.count(nextSymbol) > 0) {
                    if (isTerminal(nextSymbol)) {
                        // ʹ�� at() ���� []
                        int nextState = state.transitions.at(nextSymbol);
                        actionTable[state.stateNum][nextSymbol] =
                            "s" + std::to_string(nextState);
                    }
                    else {
                        // ʹ�� at() ���� []
                        int nextState = state.transitions.at(nextSymbol);
                        gotoTable[state.stateNum][nextSymbol] =
                            std::to_string(nextState);
                    }
                }
            }
        }
    }

    // ��ӡ������
    std::cout << "\nSLR������" << std::endl;
    printParsingTable(actionTable, gotoTable);
}

void SLRGenerator::printParsingTable(const ActionTable& actionTable,
    const GotoTable& gotoTable) {
    // ��ȡ�����ս���ͷ��ս��
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

    // ��ӡ��ͷ
    std::cout << "״̬\t";
    for (const auto& term : terminals) {
        std::cout << term << "\t";
    }
    for (const auto& nonTerm : nonTerminals) {
        if (nonTerm != "S'") {
            std::cout << nonTerm << "\t";
        }
    }
    std::cout << std::endl;

    // ��ӡÿһ��
    for (const auto& state : states) {
        std::cout << state.stateNum << "\t";

        // ��ӡACTION����
        for (const auto& term : terminals) {
            if (actionTable.count(state.stateNum) &&
                actionTable.at(state.stateNum).count(term)) {
                std::cout << actionTable.at(state.stateNum).at(term);
            }
            std::cout << "\t";
        }

        // ��ӡGOTO����
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
    std::cout << "�����������ʽSLR������..." << std::endl;
    initArithmeticGrammar();//��ʼ���ķ���������ͬ
    generateParsingTable();
}

void SLRGenerator::generateBooleanTable() {
    std::cout << "���ɲ������ʽSLR������..." << std::endl;
    initBooleanGrammar();
    generateParsingTable();
}

void SLRGenerator::generateStatementTable() {
    std::cout << "���ɳ������SLR������..." << std::endl;
    initStatementGrammar();
    generateParsingTable();
}