#include "Token.h"
#include <fstream>
#include <sstream>

std::vector<Token> readTokens(const std::string& filename) {
    std::ifstream in(filename);
    std::vector<Token> tokens;
    std::string lineStr;

    while (std::getline(in, lineStr)) {
        if (lineStr.rfind("Line ", 0) != 0) continue;

        std::string dummy, colon, category, arrow;
        int lineNo;

        std::stringstream ss(lineStr);
        ss >> dummy >> lineNo >> colon;
        ss >> category >> arrow;

        std::string lexeme;
        std::getline(ss, lexeme);
        if (!lexeme.empty() && lexeme[0] == ' ') lexeme.erase(0, 1);

        if (category == "PREPROCESSOR" || category == "COMMENT") continue;

        tokens.push_back({ category, lexeme, lineNo });
    }

    tokens.push_back({ "EOF", "EOF", -1 });
    return tokens;
}
