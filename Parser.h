#pragma once
#include <vector>
#include <string>
#include "Token.h"
#include "Node.h"

class Parser {
public:
    explicit Parser(const std::vector<Token>& toks);
    Node* parseProgram();

private:
    std::vector<Token> tokens;
    size_t pos;

    const Token& peek(int offset = 0) const;
    const Token& advance();
    const Token& expectLexeme(const std::string& lex);
    const Token& expectCategory(const std::string& cat);
    bool matchLexeme(const std::string& lex);
    bool checkLexeme(const std::string& lex, int offset = 0) const;
    bool checkCategory(const std::string& cat, int offset = 0) const;

    Node* parseExternalDecl();
    Node* parseStructDecl();
    Node* parseFunctionDecl();
    Node* parseTypeSpec();
    Node* parseParamListOpt();
    Node* parseParamList();
    Node* parseParam();
    Node* parseCompoundStmt();
    Node* parseStmtList();
    Node* parseStmt();
    Node* parseIfStmt();
    Node* parseForStmt();
    Node* parseReturnStmt();
    Node* parseDeclStmt(bool withSemi);
    Node* parseExprStmt();
    Node* parseExpr();
    Node* parseAssignExpr();
    Node* parseOrExpr();
    Node* parseAndExpr();
    Node* parseEqualityExpr();
    Node* parseRelExpr();
    Node* parseAddExpr();
    Node* parseMulExpr();
    Node* parseUnaryExpr();
    Node* parsePostfixExpr();
    Node* parsePrimary();
};
