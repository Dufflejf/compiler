#pragma once
#include <string>
#include <vector>

struct Production {
    std::string left;
    std::vector<std::string> right;

    Production(const std::string& l, const std::vector<std::string>& r)
        : left(l), right(r) {
    }

    bool operator==(const Production& other) const {
        return left == other.left && right == other.right;
    }

    bool operator<(const Production& other) const {
        if (left != other.left) return left < other.left;
        return right < other.right;
    }
};