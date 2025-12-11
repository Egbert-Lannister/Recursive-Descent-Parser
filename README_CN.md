# 递归下降语法分析器

## 1. 项目概述

本项目基于 Lab1 的词法分析结果，实现了一个针对 mini-C 语言子集的
**递归下降（recursive descent）语法分析器**。  
程序从 Lab1 生成的 token 文件中读取记号序列，对其进行自顶向下的语法分析，
构建完整的语法树（parse tree），并以缩进文本形式输出。

支持的主要语言特性包括：

- 函数声明与定义（`int f(int a, int b)` 等）
- `struct` 结构体定义及成员声明
- 复合语句与语句序列（`{ ... }`）
- 条件语句 `if / else`
- 循环语句 `for`
- 变量声明与初始化（含数组、指针、`const`）
- 表达式及运算符优先级：
  - 赋值：`=`, `+=`
  - 逻辑：`&&`, `||`
  - 关系：`<`, `>`, `<=`, `>=`, `==`
  - 算术：`+`, `-`, `*`, `/`, `%`
  - 一元：`+`, `-`, `!`
  - 后缀运算：函数调用、数组下标、字段访问（`.`）、`++`
- 标识符、整型常量、字符常量、字符串常量等基本记号

---

## 2. 文件结构

项目采用多文件模块化结构，各文件功能如下：

### 2.1 头文件（Header Files）

- `Token.h`  
  - 定义 `struct Token`：
    - `std::string category;`  // 记号类型（如 KEYWORD, IDENTIFIER）
    - `std::string lexeme;`    // 词素内容（如 "int", "a", "=="）
    - `int line;`              // 源代码所在行号  
  - 声明函数：
    - `std::vector<Token> readTokens(const std::string &filename);`  
      从 Lab1 输出的 token 文件中读取所有记号。

- `Node.h`  
  - 定义语法树节点结构 `struct Node`：
    - `std::string symbol;`    // 该结点表示的文法符号/标签
    - `std::vector<Node*> children;`  // 子结点列表
  - 声明：
    - `void printTree(Node* node, int indent = 0);`  
      递归打印语法树，使用缩进展示层次结构。

- `Parser.h`  
  - 定义 `class Parser`，实现递归下降语法分析器。
  - 成员主要包括：
    - `std::vector<Token> tokens;`  // 输入 token 序列
    - `size_t pos;`                 // 当前读取位置
  - 工具函数（utility functions）：
    - `const Token& peek(int offset = 0) const;`
    - `const Token& advance();`
    - `bool checkLexeme(const std::string &lex, int offset = 0) const;`
    - `bool checkCategory(const std::string &cat, int offset = 0) const;`
    - `bool matchLexeme(const std::string &lex);`
    - `const Token& expectLexeme(const std::string &lex);`
    - `const Token& expectCategory(const std::string &cat);`
  - 语法分析函数（每个非终结符对应一个 `parseXXX()`）：
    - `parseProgram()`, `parseExternalDecl()`
    - `parseStructDecl()`, `parseFunctionDecl()`, `parseTypeSpec()`
    - `parseParamListOpt()`, `parseParamList()`, `parseParam()`
    - `parseCompoundStmt()`, `parseStmtList()`, `parseStmt()`
    - `parseIfStmt()`, `parseForStmt()`, `parseReturnStmt()`
    - `parseDeclStmt(bool withSemi)`, `parseExprStmt()`
    - 表达式相关：`parseExpr()`, `parseAssignExpr()`,
      `parseOrExpr()`, `parseAndExpr()`, `parseEqualityExpr()`,
      `parseRelExpr()`, `parseAddExpr()`, `parseMulExpr()`,
      `parseUnaryExpr()`, `parsePostfixExpr()`, `parsePrimary()`

### 2.2 源文件（Source Files）

- `Token.cpp`  
  实现 `readTokens()`：  
  - 按行读取 token 文件（形如 `Line 7: KEYWORD -> int`）  
  - 解析行号、类别、词素  
  - 忽略 `PREPROCESSOR` 和 `COMMENT`  
  - 将其余 token 存入 `vector<Token>`  
  - 最后追加一个 `{"EOF", "EOF", -1}` 作为结束标记。

- `Node.cpp`  
  实现 `printTree()`：  
  - 使用 `indent` 控制缩进层数，每层两个空格；  
  - 先输出当前结点的 `symbol`，再递归输出子结点。

- `Parser.cpp`  
  - 实现所有 `Parser` 成员函数。  
  - 以递归下降方式，对照文法依次解析程序、声明、语句与表达式，构建语法树。  
  - 当遇到不匹配的 token 时，通过 `expectLexeme` / `expectCategory` 输出错误信息并退出。

- `main.cpp`  
  程序入口：
  ```cpp
  int main() {
      auto tokens = readTokens("tokens.txt");
      Parser parser(tokens);
      Node* root = parser.parseProgram();
      printTree(root);
      return 0;
  }
    ```

* `tokens.txt`
  来自 Lab1 的 token 输出文件，是本项目的输入。

---

## 3. 编译与运行说明

### 3.1 编译

在 Visual Studio 中：

1. 使用已有的 `Lab2` 解决方案和项目。
2. 确保上述 `.h` / `.cpp` 文件全部已添加到项目。
3. 将 `tokens.txt` 放在**程序工作目录**下：

   * 默认为 `$(ProjectDir)` 或编译输出目录（如 `x64/Debug/`），
     可在「项目属性 → 调试 → 工作目录」中设置。
4. 生成解决方案（`Ctrl + Shift + B`）。

### 3.2 运行

* 使用 `Ctrl + F5` 运行程序。

* 控制台会输出整棵语法树，例如：

  ```text
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

* 如需保存到文件，可在命令行运行可执行文件并重定向输出：

  ```bash
  Lab2.exe > parse_tree.txt
  ```

---

## 4. 关键数据结构说明

### 4.1 Token

```cpp
struct Token {
    std::string category;  // 记号类别，如 "KEYWORD", "IDENTIFIER", "NUMBER"
    std::string lexeme;    // 词素内容，如 "int", "a", "=="
    int line;              // 源代码行号
};
```

* `category` 对应 Lab1 中的 token 类型。
* `lexeme` 是在语法分析中进行匹配的主要依据（如匹配 `if`, `for`, `(` 等）。

### 4.2 Node（语法树节点）

```cpp
struct Node {
    std::string symbol;           // 文法符号/结点标签，如 "IfStmt", "ID(a)"
    std::vector<Node*> children;  // 子结点
    Node(const std::string &s) : symbol(s) {}
};
```

* `symbol` 通常是非终结符名或带信息的叶子节点：

  * 非终结符：`"IfStmt"`, `"ForStmt"`, `"DeclStmt"`…
  * 终结符/叶子：`"ID(a)"`, `"NUM(0)"`, `"STR(\"%c \")"` 等。
* 利用 `children` 构成树结构，根结点为 `"Program"`。

---

## 5. 语法分析流程说明

### 5.1 顶层：程序与外部声明

* `parseProgram()`

  * 创建根结点 `"Program"`；
  * 在未遇到 `EOF` 前，不断调用 `parseExternalDecl()`；
  * 每个外部声明要么是函数定义，要么是 `struct` 定义。

* `parseExternalDecl()`

  * 如果当前 token 形如 `struct IDENTIFIER {`，调用 `parseStructDecl()`；
  * 否则默认按照函数定义解析，调用 `parseFunctionDecl()`。

### 5.2 函数与结构体

* `parseFunctionDecl()`

  * 按顺序解析：返回类型 `TypeSpec` → 函数名 → 参数列表 → 函数体 `{ ... }`。
  * 对应文法：
    `FunctionDecl → TypeSpec IDENTIFIER "(" ParamListOpt ")" CompoundStmt`。

* `parseStructDecl()`

  * 解析 `struct 名称 { 成员声明列表 };`
  * 成员声明统一复用 `parseDeclStmt(true)`。

### 5.3 语句层（Statement Level）

* `parseStmt()` 根据当前 token 决定分派：

  * `if` → `parseIfStmt()`
  * `for` → `parseForStmt()`
  * `return` → `parseReturnStmt()`
  * `{` → `parseCompoundStmt()`
  * 类型关键字 / `struct` / `const` → `parseDeclStmt(true)`
  * 其他 → 默认视为表达式语句 `parseExprStmt()`（表达式 + `;`）

* `parseIfStmt()`
  实现 `if (Expr) Stmt [else Stmt]`，支持嵌套与 else-if 链。

* `parseForStmt()`
  实现 `for (init; cond; iter) Stmt`，其中 init / cond / iter 都可为空：

  * init 可以是声明（如 `int i = 0`）或普通表达式；
  * cond、iter 若为空，则解析为 ε。

### 5.4 声明（Declaration）

* `parseDeclStmt(withSemi)`

  * 解析 `TypeSpec` + 一个或多个 `Declarator`（逗号分隔）；
  * 支持形式：`int a;`、`int a = 0, b = 1;`、`char name[50];` 等；
  * `withSemi == true` 时，期望最后有 `;`；
  * `for` 语句中的初始化子句则使用 `withSemi == false`。

### 5.5 表达式与运算符优先级

表达式解析采用“每一层一个函数”的方式，从最低优先级到最高优先级：

* `parseExpr()` → `parseAssignExpr()`
* `parseAssignExpr()`：处理 `=` 和 `+=`，左边要求是 `PostfixExpr`；
* `parseOrExpr()`：处理 `||`
* `parseAndExpr()`：处理 `&&`
* `parseEqualityExpr()`：处理 `==`
* `parseRelExpr()`：处理 `<`, `>`, `<=`, `>=`
* `parseAddExpr()`：处理 `+`、`-`（注意排除 `+=`）
* `parseMulExpr()`：处理 `*`、`/`、`%`
* `parseUnaryExpr()`：处理一元 `+`、`-`、`!`
* `parsePostfixExpr()`：处理后缀操作：

  * 函数调用：`ID(...)`
  * 数组下标：`ID[...]`
  * 字段访问：`expr.ID`
  * 自增：`expr++`
* `parsePrimary()`：基本单元：

  * `IDENTIFIER`、`NUMBER`、`CHAR_CONSTANT`、`STRING`
  * `(` Expr `)`

这样设计保证了表达式按照正确优先级和结合性构建语法树，
例如：

* `a >= b && b == c`
* `sum += arr[i]`
* `printf("%c ", letters[i]);`

都会生成结构正确的子树，在实验结果中已经验证。

---

## 6. 错误处理机制

* 所有需要“必须匹配某个符号”的地方，使用 `expectLexeme()` 或
  `expectCategory()`：

  * 若当前 token 不符合预期，则输出错误信息并退出程序：

    ```cpp
    Syntax error at line 11: expected ')' but got '>='
    ```
* 这种设计可以快速定位语法错误，便于调试文法与代码。
* 对于 `matchLexeme()`，仅在“可选或重复出现”的位置使用，
  不匹配时不会报错，只是返回 `false`。

---

## 7. 扩展与修改建议

如果后续需要扩展本语法分析器，可以考虑：

* 增加更多语言特性：

  * `while` / `do-while` 循环
  * `switch` 语句
  * 函数指针、枚举等
* 在 `Node` 中增加更多语义信息（如类型、行号），为以后语义分析做准备；
* 将当前的 parse tree 进一步抽象成 AST（去掉多余的中间结点）；
* 改进错误恢复能力，例如遇到错误后跳过到下一个 `;` 或 `}`，继续分析后续语句。


