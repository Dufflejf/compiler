#pragma once
#include "production.h"
#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>

struct LR0Item {
    Production prod;
    int dotPos;

    LR0Item(const Production& p, int pos) : prod(p), dotPos(pos) {}

    bool operator==(const LR0Item& other) const {
        return prod.left == other.prod.left &&
            prod.right == other.prod.right &&
            dotPos == other.dotPos;
    }

    bool operator<(const LR0Item& other) const {
        if (prod.left != other.prod.left) return prod.left < other.prod.left;
        if (prod.right != other.prod.right) return prod.right < other.prod.right; // 修正这里
        return dotPos < other.dotPos;
    }
};

// 状态类定义
struct State {
    std::set<LR0Item> items;
    std::map<std::string, int> transitions;
    int stateNum;

    State(const std::set<LR0Item>& i, int num) : items(i), stateNum(num) {}

    bool operator<(const State& other) const {
        return stateNum < other.stateNum;
    }
};
