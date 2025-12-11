#include "Node.h"
#include <iostream>

void printTree(Node* node, int indent) {
    for (int i = 0; i < indent; ++i) std::cout << "  ";
    std::cout << node->symbol << "\n";

    for (auto* child : node->children) {
        printTree(child, indent + 1);
    }
}
