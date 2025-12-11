#include "Parser.h"
#include "Token.h"
#include "Node.h"

int main() {
    auto tokens = readTokens("D:/C_Script/Lab2/token_analysis_result.txt");
    Parser parser(tokens);
    Node* root = parser.parseProgram();
    printTree(root);
    return 0;
}
