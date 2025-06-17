#pragma once
#include <string>
#include <vector>

struct Node {
    std::string type;
    std::string value;
    std::vector<Node*> children;

    Node(const std::string& t, const std::string& v = "")
        : type(t), value(v) {
    }

    ~Node() {
        for (auto child : children) {
            delete child;
        }
    }
};
