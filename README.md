# Recursive-Descent Parser

## 1. Project Overview

Building on the lexical-analysis results of Lab1, this project implements a **recursive-descent parser** for a mini-C language subset.  
The program reads the token sequence produced by Lab1, performs top-down syntactic analysis, constructs a complete parse tree, and prints it in indented textual form.

Supported language constructs include:

- Function declarations and definitions (`int f(int a, int b)`, etc.)
- `struct` definitions and member declarations
- Compound statements and statement sequences (`{ ... }`)
- Conditional statements `if / else`
- Loop statements `for`
- Variable declarations and initializations (arrays, pointers, `const`)
- Expressions and operator precedence:
  - Assignment: `=`, `+=`
  - Logical: `&&`, `||`
  - Relational: `<`, `>`, `<=`, `>=`, `==`
  - Arithmetic: `+`, `-`, `*`, `/`, `%`
  - Unary: `+`, `-`, `!`
  - Postfix: function calls, array subscripts, field access (`.`), `++`
- Basic tokens: identifiers, integer literals, character literals, string literals, etc.

---

## 2. File Organization

The project adopts a multi-file modular structure; each file's role is summarized below.

### 2.1 Header Files

- `Token.h`  
  - Defines `struct Token`:
    - `std::string category;`  // token type (e.g., KEYWORD, IDENTIFIER)
    - `std::string lexeme;`    // spelling (e.g., "int", "a", "==")
    - `int line;`              // source line number
  - Declares:
    - `std::vector<Token> readTokens(const std::string &filename);`  
      Reads all tokens from Lab1's output file.

- `Node.h`  
  - Defines `struct Node`:
    - `std::string symbol;`               // grammar symbol / label
    - `std::vector<Node*> children;`       // child pointers
  - Declares:
    - `void printTree(Node* node, int indent = 0);`  
      Recursively prints the parse tree with indentation.

- `Parser.h`  
  - Defines `class Parser`, the recursive-descent parser.
  - Main members:
    - `std::vector<Token> tokens;`
    - `size_t pos;`                 // current lookahead index
  - Utility methods:
    - `const Token& peek(int offset = 0) const;`
    - `const Token& advance();`
    - `bool checkLexeme(const std::string &lex, int offset = 0) const;`
    - `bool checkCategory(const std::string &cat, int offset = 0) const;`
    - `bool matchLexeme(const std::string &lex);`
    - `const Token& expectLexeme(const std::string &lex);`
    - `const Token& expectCategory(const std::string &cat);`
  - Parsing routines (one per non-terminal):
    - `parseProgram()`, `parseExternalDecl()`
    - `parseStructDecl()`, `parseFunctionDecl()`, `parseTypeSpec()`
    - `parseParamListOpt()`, `parseParamList()`, `parseParam()`
    - `parseCompoundStmt()`, `parseStmtList()`, `parseStmt()`
    - `parseIfStmt()`, `parseForStmt()`, `parseReturnStmt()`
    - `parseDeclStmt(bool withSemi)`, `parseExprStmt()`
    - Expression hierarchy: `parseExpr()`, `parseAssignExpr()`,
      `parseOrExpr()`, `parseAndExpr()`, `parseEqualityExpr()`,
      `parseRelExpr()`, `parseAddExpr()`, `parseMulExpr()`,
      `parseUnaryExpr()`, `parsePostfixExpr()`, `parsePrimary()`

### 2.2 Source Files

- `Token.cpp`  
  Implements `readTokens()`:
  - Reads the token file line-by-line (format: `Line 7: KEYWORD -> int`)
  - Extracts line number, category, lexeme
  - Skips `PREPROCESSOR` and `COMMENT`
  - Appends `{"EOF", "EOF", -1}` as end-of-stream marker

- `Node.cpp`  
  Implements `printTree()`:
  - Uses `indent` level (two spaces per level)
  - Prints current node's `symbol`, then recursively prints children

- `Parser.cpp`  
  - Implements all `Parser` members
  - Performs recursive-descent parsing, builds the parse tree
  - Reports syntax errors via `expectLexeme` / `expectCategory` and terminates

- `main.cpp`  
  Entry point:
  ```cpp
  int main() {
      auto tokens = readTokens("tokens.txt");
      Parser parser(tokens);
      Node* root = parser.parseProgram();
      printTree(root);
      return 0;
  }
  ```

- `tokens.txt`  
  Token stream produced by Lab1; the parser's input.

---

## 3. Compilation & Execution

### 3.1 Compilation (Visual Studio)

1. Open the provided `Lab2` solution / project
2. Ensure all `.h` / `.cpp` files are included
3. Place `tokens.txt` in the **working directory** (default: `$(ProjectDir)` or `x64/Debug/`; configurable via Project Properties → Debugging → Working Directory)
4. Build solution (`Ctrl + Shift + B`)

### 3.2 Execution

- Run with `Ctrl + F5`
- Console displays the entire parse tree, e.g.:

  ```
  Program
    FunctionDecl
      TypeSpec
        BaseType(int)
      FuncName(compareThreeNumbers)
      ...
    StructDecl
      StructName(Student)
      ...
    FunctionDecl
      FuncName(safeStringCopy)
      ...
  ```

- To save output:

  ```bash
  Lab2.exe > parse_tree.txt
  ```

---

## 4. Key Data Structures

### 4.1 Token

```cpp
struct Token {
    std::string category;  // token type, e.g., "KEYWORD", "IDENTIFIER"
    std::string lexeme;    // spelling, e.g., "int", "a", "=="
    int line;              // source line number
};
```

- `category` corresponds to Lab1's token types
- `lexeme` is the primary value used for parsing decisions

### 4.2 Node (Parse-Tree Node)

```cpp
struct Node {
    std::string symbol;           // grammar symbol / label, e.g., "IfStmt", "ID(a)"
    std::vector<Node*> children;  // sub-trees
    Node(const std::string &s) : symbol(s) {}
};
```

- `symbol` may be a non-terminal (`"IfStmt"`, `"ForStmt"`, `"DeclStmt"`, ...)  
  or a decorated terminal (`"ID(a)"`, `"NUM(0)"`, `"STR(\"%c \")"`, ...)
- `children` forms the tree structure; root is labeled `"Program"`

---

## 5. Parsing Workflow

### 5.1 Top Level: Program & External Declarations

- `parseProgram()`
  - Creates root node `"Program"`
  - Repeatedly calls `parseExternalDecl()` until `EOF`
- `parseExternalDecl()`
  - If current tokens match `struct IDENTIFIER {`, delegates to `parseStructDecl()`
  - Otherwise treats as function definition via `parseFunctionDecl()`

### 5.2 Functions & Structs

- `parseFunctionDecl()`
  - Parses: return type `TypeSpec` → function name → parameter list → body `{ ... }`
  - Grammar sketch:  
    `FunctionDecl → TypeSpec IDENTIFIER "(" ParamListOpt ")" CompoundStmt`
- `parseStructDecl()`
  - Parses `struct name { member-declarations };`
  - Members are parsed with `parseDeclStmt(true)`

### 5.3 Statement Level

- `parseStmt()` dispatches on lookahead:
  - `if` → `parseIfStmt()`
  - `for` → `parseForStmt()`
  - `return` → `parseReturnStmt()`
  - `{` → `parseCompoundStmt()`
  - type keyword / `struct` / `const` → `parseDeclStmt(true)`
  - otherwise → `parseExprStmt()` (expression followed by `;`)
- `parseIfStmt()`
  - Implements `if (Expr) Stmt [else Stmt]`, supporting nested if-else chains
- `parseForStmt()`
  - Implements `for (init; cond; iter) Stmt`
  - init may be a declaration (`int i = 0`) or an expression; cond / iter may be empty (ε)

### 5.4 Declarations

- `parseDeclStmt(withSemi)`
  - Parses `TypeSpec` followed by one or more declarators (comma-separated)
  - Examples: `int a;`, `int a = 0, b = 1;`, `char name[50];`
  - When `withSemi == true`, consumes final `;`; `for`-init uses `withSemi == false`

### 5.5 Expressions & Precedence

Expression parsing uses one function per precedence level (lowest to highest):

- `parseExpr()` → `parseAssignExpr()`
- `parseAssignExpr()` handles `=` and `+=`; LHS must be a `PostfixExpr`
- `parseOrExpr()` → `||`
- `parseAndExpr()` → `&&`
- `parseEqualityExpr()` → `==`
- `parseRelExpr()` → `<`, `>`, `<=`, `>=`
- `parseAddExpr()` → `+`, `-` (excluding `+=`)
- `parseMulExpr()` → `*`, `/`, `%`
- `parseUnaryExpr()` → unary `+`, `-`, `!`
- `parsePostfixExpr()` → postfix operations:
  - Function calls: `ID(...)`
  - Array subscripts: `ID[...]`
  - Field access: `expr.ID`
  - Post-increment: `expr++`
- `parsePrimary()` → basic atoms:
  - `IDENTIFIER`, `NUMBER`, `CHAR_CONSTANT`, `STRING`
  - `(` Expr `)`

This hierarchy guarantees correct precedence and associativity, e.g.:

- `a >= b && b == c`
- `sum += arr[i]`
- `printf("%c ", letters[i]);`

all yield properly structured sub-trees, validated in the experimental results.

---

## 6. Error-Handling Strategy

- Any construct that **must** match a specific token uses `expectLexeme()` or `expectCategory()`:
  - On mismatch, an error message is printed and the program terminates, e.g.:

    ```
    Syntax error at line 11: expected ')' but got '>='
    ```
- This design quickly pinpoints syntax errors, facilitating grammar/code debugging
- `matchLexeme()` is used only in optional/repetition contexts; mismatch returns `false` without error

---

## 7. Extension & Modification Hints

Future enhancements may include:

- Additional language features:
  - `while` / `do-while` loops
  - `switch` statements
  - Function pointers, enumerations, etc.
- Enriching `Node` with semantic information (types, line numbers) for later semantic analysis
- Converting the current parse tree into a concise AST (eliding redundant internal nodes)
- Improving error recovery, e.g., skip to next `;` or `}` upon error and continue parsing subsequent statements