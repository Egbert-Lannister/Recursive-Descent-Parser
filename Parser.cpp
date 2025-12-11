#include "Parser.h"
#include <iostream>
#include <cstdlib>

// ===== Constructor =====
Parser::Parser(const std::vector<Token>& toks) : tokens(toks), pos(0) {}


// ===== Helper Functions =====

const Token& Parser::peek(int offset) const {
    size_t index = pos + offset;
    if (index >= tokens.size()) return tokens.back();
    return tokens[index];
}

const Token& Parser::advance() {
    return tokens[pos++];
}

bool Parser::checkLexeme(const std::string& lex, int offset) const {
    return peek(offset).lexeme == lex;
}

bool Parser::checkCategory(const std::string& cat, int offset) const {
    return peek(offset).category == cat;
}

const Token& Parser::expectLexeme(const std::string& lex) {
    if (!checkLexeme(lex)) {
        std::cerr << "Syntax error at line " << peek().line
            << ": expected '" << lex << "' but got '" << peek().lexeme << "'\n";
        exit(1);
    }
    return advance();
}

const Token& Parser::expectCategory(const std::string& cat) {
    if (!checkCategory(cat)) {
        std::cerr << "Syntax error at line " << peek().line
            << ": expected category '" << cat << "' but got '"
            << peek().category << "' (" << peek().lexeme << ")\n";
        exit(1);
    }
    return advance();
}

bool Parser::matchLexeme(const std::string& lex) {
    if (checkLexeme(lex)) { advance(); return true; }
    return false;
}


// ===================================================================
//               Program-Level Parsing (struct / function)
// ===================================================================

Node* Parser::parseProgram() {
    Node* node = new Node("Program");
    while (!checkLexeme("EOF")) {
        node->children.push_back(parseExternalDecl());
    }
    return node;
}

Node* Parser::parseExternalDecl() {
    // struct definition?
    if (checkLexeme("struct") && checkCategory("IDENTIFIER", 1) && checkLexeme("{", 2)) {
        return parseStructDecl();
    }

    // otherwise must be function
    return parseFunctionDecl();
}


// ===================================================================
//                           struct Declaration
// ===================================================================

Node* Parser::parseStructDecl() {
    Node* node = new Node("StructDecl");

    expectLexeme("struct");
    Token name = expectCategory("IDENTIFIER");
    node->children.push_back(new Node("StructName(" + name.lexeme + ")"));

    expectLexeme("{");

    Node* members = new Node("StructMemberList");
    while (!checkLexeme("}")) {
        members->children.push_back(parseDeclStmt(true)); // local-style "Type id;"
    }

    expectLexeme("}");
    expectLexeme(";");

    node->children.push_back(members);
    return node;
}


// ===================================================================
//                           function Declaration
// ===================================================================

Node* Parser::parseFunctionDecl() {
    Node* node = new Node("FunctionDecl");

    node->children.push_back(parseTypeSpec());

    Token fname = expectCategory("IDENTIFIER");
    node->children.push_back(new Node("FuncName(" + fname.lexeme + ")"));

    expectLexeme("(");
    node->children.push_back(parseParamListOpt());
    expectLexeme(")");

    node->children.push_back(parseCompoundStmt());
    return node;
}


// ===================================================================
//                               Types
// ===================================================================

Node* Parser::parseTypeSpec() {
    Node* node = new Node("TypeSpec");

    if (matchLexeme("const")) {
        node->children.push_back(new Node("const"));
    }

    if (checkLexeme("struct")) {
        advance();
        Token id = expectCategory("IDENTIFIER");
        node->children.push_back(new Node("struct " + id.lexeme));
        return node;
    }

    if (checkLexeme("int") || checkLexeme("char") || checkLexeme("float")
        || checkLexeme("void") || checkLexeme("size_t")) {
        Token base = advance();
        node->children.push_back(new Node("BaseType(" + base.lexeme + ")"));
        return node;
    }

    std::cerr << "Invalid type at line " << peek().line << "\n";
    exit(1);
}


// ===================================================================
//                        Parameters (inside function)
// ===================================================================

Node* Parser::parseParamListOpt() {
    if (checkLexeme(")")) {
        return new Node("ParamList(ε)");
    }
    return parseParamList();
}

Node* Parser::parseParamList() {
    Node* node = new Node("ParamList");
    node->children.push_back(parseParam());

    while (matchLexeme(",")) {
        node->children.push_back(parseParam());
    }
    return node;
}

Node* Parser::parseParam() {
    Node* node = new Node("Param");
    node->children.push_back(parseTypeSpec());

    // pointer *
    while (matchLexeme("*")) {
        node->children.push_back(new Node("*"));
    }

    Token id = expectCategory("IDENTIFIER");
    node->children.push_back(new Node("ParamName(" + id.lexeme + ")"));

    // array suffix?
    if (matchLexeme("[")) {
        node->children.push_back(new Node("["));
        if (checkCategory("NUMBER")) {
            Token n = advance();
            node->children.push_back(new Node("NUM(" + n.lexeme + ")"));
        }
        expectLexeme("]");
        node->children.push_back(new Node("]"));
    }

    return node;
}


// ===================================================================
//                      CompoundStmt / StmtList / Stmt
// ===================================================================

Node* Parser::parseCompoundStmt() {
    Node* node = new Node("CompoundStmt");
    expectLexeme("{");
    node->children.push_back(parseStmtList());
    expectLexeme("}");
    return node;
}

Node* Parser::parseStmtList() {
    Node* node = new Node("StmtList");
    while (!checkLexeme("}") && !checkLexeme("EOF")) {
        node->children.push_back(parseStmt());
    }
    return node;
}

Node* Parser::parseStmt() {
    if (checkLexeme("if")) return parseIfStmt();
    if (checkLexeme("for")) return parseForStmt();
    if (checkLexeme("return")) return parseReturnStmt();
    if (checkLexeme("{")) return parseCompoundStmt();

    // Declaration?
    if (checkLexeme("int") || checkLexeme("char") || checkLexeme("float")
        || checkLexeme("void") || checkLexeme("size_t") || checkLexeme("struct")
        || checkLexeme("const")) {
        return parseDeclStmt(true);
    }

    // Default: expression statement
    return parseExprStmt();
}


// ===================================================================
//                                 If statement
// ===================================================================

Node* Parser::parseIfStmt() {
    Node* node = new Node("IfStmt");

    expectLexeme("if");
    expectLexeme("(");
    node->children.push_back(parseExpr());
    expectLexeme(")");

    node->children.push_back(parseStmt());

    if (matchLexeme("else")) {
        Node* e = new Node("Else");
        e->children.push_back(parseStmt());
        node->children.push_back(e);
    }

    return node;
}


// ===================================================================
//                                  for ( ... )
// ===================================================================

Node* Parser::parseForStmt() {
    Node* node = new Node("ForStmt");

    expectLexeme("for");
    expectLexeme("(");

    // init
    if (!checkLexeme(";")) {
        if (checkLexeme("int") || checkLexeme("char") || checkLexeme("float")
            || checkLexeme("void") || checkLexeme("size_t") || checkLexeme("struct")
            || checkLexeme("const")) {
            node->children.push_back(parseDeclStmt(false));
        }
        else {
            Node* init = new Node("ForInitExpr");
            init->children.push_back(parseExpr());
            node->children.push_back(init);
        }
    }

    expectLexeme(";");

    // cond
    if (!checkLexeme(";")) {
        node->children.push_back(parseExpr());
    }
    expectLexeme(";");

    // iter
    if (!checkLexeme(")")) {
        Node* iter = new Node("ForIterExpr");
        iter->children.push_back(parseExpr());
        node->children.push_back(iter);
    }

    expectLexeme(")");
    node->children.push_back(parseStmt());

    return node;
}


// ===================================================================
//                               return x;
// ===================================================================

Node* Parser::parseReturnStmt() {
    Node* node = new Node("ReturnStmt");

    expectLexeme("return");

    if (!checkLexeme(";")) {
        node->children.push_back(parseExpr());
    }

    expectLexeme(";");
    return node;
}


// ===================================================================
//                declarations: int x = 0, y = 2;
// ===================================================================

Node* Parser::parseDeclStmt(bool withSemi) {
    Node* node = new Node("DeclStmt");

    node->children.push_back(parseTypeSpec());

    do {
        Node* decl = new Node("Declarator");

        while (matchLexeme("*")) {
            decl->children.push_back(new Node("*"));
        }

        Token id = expectCategory("IDENTIFIER");
        decl->children.push_back(new Node("Var(" + id.lexeme + ")"));

        if (matchLexeme("[")) {
            decl->children.push_back(new Node("["));
            if (checkCategory("NUMBER")) {
                Token n = advance();
                decl->children.push_back(new Node("NUM(" + n.lexeme + ")"));
            }
            expectLexeme("]");
            decl->children.push_back(new Node("]"));
        }

        if (matchLexeme("=")) {
            decl->children.push_back(new Node("="));
            decl->children.push_back(parseExpr());
        }

        node->children.push_back(decl);

    } while (matchLexeme(","));

    if (withSemi) expectLexeme(";");

    return node;
}


// ===================================================================
//                             Expression Statement
// ===================================================================

Node* Parser::parseExprStmt() {
    Node* node = new Node("ExprStmt");
    node->children.push_back(parseExpr());
    expectLexeme(";");
    return node;
}


// ===================================================================
//                               Expressions
// ===================================================================

Node* Parser::parseExpr() {
    return parseAssignExpr();
}


// assignment:   a = b   or a += b (tokenized as '+' '=')
Node* Parser::parseAssignExpr() {
    Node* left = parseOrExpr();

    if (checkLexeme("=") ||
        (checkLexeme("+") && checkLexeme("=", 1))) {

        Node* node = new Node("AssignExpr");
        node->children.push_back(left);

        if (checkLexeme("+") && checkLexeme("=", 1)) {
            advance();
            advance();
            node->children.push_back(new Node("+="));
        }
        else {
            advance();
            node->children.push_back(new Node("="));
        }

        node->children.push_back(parseAssignExpr());
        return node;
    }

    return left;
}


Node* Parser::parseOrExpr() {
    Node* node = parseAndExpr();

    while (matchLexeme("||")) {
        Node* parent = new Node("OrExpr");
        parent->children.push_back(node);
        parent->children.push_back(new Node("||"));
        parent->children.push_back(parseAndExpr());
        node = parent;
    }

    return node;
}


Node* Parser::parseAndExpr() {
    Node* node = parseEqualityExpr();

    while (matchLexeme("&&")) {
        Node* parent = new Node("AndExpr");
        parent->children.push_back(node);
        parent->children.push_back(new Node("&&"));
        parent->children.push_back(parseEqualityExpr());
        node = parent;
    }

    return node;
}


Node* Parser::parseEqualityExpr() {
    Node* node = parseRelExpr();

    while (matchLexeme("==")) {
        Node* parent = new Node("EqExpr");
        parent->children.push_back(node);
        parent->children.push_back(new Node("=="));
        parent->children.push_back(parseRelExpr());
        node = parent;
    }

    return node;
}


Node* Parser::parseRelExpr() {
    Node* node = parseAddExpr();

    while (checkLexeme("<") || checkLexeme(">") ||
        checkLexeme("<=") || checkLexeme(">=")) {

        Token op = advance();
        Node* parent = new Node("RelExpr");
        parent->children.push_back(node);
        parent->children.push_back(new Node(op.lexeme));
        parent->children.push_back(parseAddExpr());
        node = parent;
    }

    return node;
}



Node* Parser::parseAddExpr() {
    Node* node = parseMulExpr();

    // 只在 "+" 不是 "+="（即后面不是 "="）时，才当作加号
    while ((checkLexeme("+") && !checkLexeme("=", 1))   // 单独的 '+'
        || checkLexeme("-")) {                        // 或者 '-'
        Token op = advance();                            // 吃掉 '+' 或 '-'
        Node* parent = new Node("AddExpr");
        parent->children.push_back(node);
        parent->children.push_back(new Node(op.lexeme));
        parent->children.push_back(parseMulExpr());
        node = parent;
    }

    return node;
}


Node* Parser::parseMulExpr() {
    Node* node = parseUnaryExpr();

    while (checkLexeme("*") || checkLexeme("/") || checkLexeme("%")) {
        Token op = advance();
        Node* parent = new Node("MulExpr");
        parent->children.push_back(node);
        parent->children.push_back(new Node(op.lexeme));
        parent->children.push_back(parseUnaryExpr());
        node = parent;
    }

    return node;
}


Node* Parser::parseUnaryExpr() {
    if (checkLexeme("+") || checkLexeme("-") || checkLexeme("!")) {
        Token op = advance();
        Node* node = new Node("UnaryExpr");
        node->children.push_back(new Node(op.lexeme));
        node->children.push_back(parseUnaryExpr());
        return node;
    }
    return parsePostfixExpr();
}


// postfix: f(x), a[i], x++, struct.field
Node* Parser::parsePostfixExpr() {
    Node* node = parsePrimary();

    while (true) {
        if (matchLexeme("[")) {
            Node* parent = new Node("ArrayAccess");
            parent->children.push_back(node);
            parent->children.push_back(parseExpr());
            expectLexeme("]");
            node = parent;
        }
        else if (matchLexeme("(")) {
            Node* parent = new Node("FuncCall");
            parent->children.push_back(node);

            Node* args = new Node("Args");
            if (!checkLexeme(")")) {
                args->children.push_back(parseExpr());
                while (matchLexeme(",")) {
                    args->children.push_back(parseExpr());
                }
            }

            expectLexeme(")");
            parent->children.push_back(args);
            node = parent;
        }
        else if (matchLexeme(".")) {
            Token field = expectCategory("IDENTIFIER");
            Node* parent = new Node("FieldAccess");
            parent->children.push_back(node);
            parent->children.push_back(new Node("Field(" + field.lexeme + ")"));
            node = parent;
        }
        else if (matchLexeme("++")) {
            Node* parent = new Node("PostInc");
            parent->children.push_back(node);
            node = parent;
        }
        else break;
    }

    return node;
}


// Primary values
Node* Parser::parsePrimary() {
    if (matchLexeme("(")) {
        Node* node = parseExpr();
        expectLexeme(")");
        return node;
    }

    if (checkCategory("IDENTIFIER")) {
        Token id = advance();
        return new Node("ID(" + id.lexeme + ")");
    }

    if (checkCategory("NUMBER")) {
        Token n = advance();
        return new Node("NUM(" + n.lexeme + ")");
    }

    if (checkCategory("CHAR_CONSTANT")) {
        Token c = advance();
        return new Node("CHAR(" + c.lexeme + ")");
    }

    if (checkCategory("STRING")) {
        Token s = advance();
        return new Node("STR(" + s.lexeme + ")");
    }

    std::cerr << "Unexpected token in primary: " << peek().lexeme
        << " at line " << peek().line << "\n";
    exit(1);
}
