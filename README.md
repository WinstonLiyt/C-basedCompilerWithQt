## C-basedCompilerWithQt :wink:

### Project Description
![main](https://github.com/WinstonLiyt/C-basedCompilerWithQt/assets/104308117/c07469ff-ae13-43a6-9c45-f69d48d4d218)

Using a high-level programming language, implement a **one-pass compiler** that resembles C. The specific requirements are as follows:

1. **Construct a lexical analysis DFA** to recognize character types in the code and form basic tokens to be sent to the syntax analyzer.
2. Based on the **LR(1) analysis method**, write a syntax analysis program that, given an input grammar, **automatically generates the ACTION and GOTO tables** and performs reduction and judgment.
3. The **syntax analysis program** should be able to call the lexical analysis program and take into account the subsequent semantic analysis module.
4. For a given grammar and a segment of code, the program should **correctly determine whether the code string is a sentence of the grammar** and output the analysis process and syntax tree.
5. Generate corresponding **quadruples for each syntax structure** (e.g., assignment statements, conditional statements, loop statements, function jumps).
6. Handle the **backpatching of quadruples** (e.g., jump targets in conditional jump statements).
7. Perform simple **optimizations**, such as eliminating redundant temporary variables.
8. **Output the generated quadruples to a file**.
9. Based on the result of the intermediate code generation, use **live variable information and register allocation algorithms** to generate assembly target code. If successful, **generate MIPS assembly code**.
10. **Run the assembly code through Mars** and verify that the results of the assembly code match the results of the C++ code.
