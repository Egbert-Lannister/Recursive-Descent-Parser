#pragma once
#include <string>
#include <vector>

struct Token {
    std::string category;
    std::string lexeme;
    int line;
};

std::vector<Token> readTokens(const std::string& filename);
