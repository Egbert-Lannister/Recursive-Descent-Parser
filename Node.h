#pragma once
#include <string>
#include <vector>

struct Node {
    std::string symbol;
    std::vector<Node*> children;
    Node(const std::string& s) : symbol(s) {}
};

void printTree(Node* node, int indent = 0);
