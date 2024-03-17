#pragma once
#ifndef _SOURCE_H
#define _SOURCE_H

#include <string>
#include <vector>
#include <list>
#include <QtWidgets/QMainWindow>
#include <QString>
#include <QTextBrowser>

using namespace std;

// ---------------- 注意事项 ----------------
// 1.暂时未考虑float型，若考虑将修改产生式
// 2.暂时未考虑数组，若考虑将修改产生式
// 3.暂时未考虑宏定义，若考虑将在词法分析后处理宏定义

// ---------------- 命名规则 ----------------
// 1.变量、结构体:驼峰法
// 2.函数:下划线、小写
// 3.类:下划线、大写
// 4.宏定义:全大写
// 5.资源文件:全大写

extern string Message;

// ---------------- 资源文件 ----------------
// 终结符列表
#define VTSIZE 85
static const char *VTNAMES[] = {
    // 类型
    "Null",       // 初始化无类型 - 0
    "Integer",    // 整型
    "String",     // 字符串
    "Boolean",    // 布尔型
    "Floating",   // 浮点型
    "Character",  // 字符
    "Comment",    // 注释
    "Macro",      // 宏定义
    "WhiteSpace", // 空格
    "EndLine",    // 换行
    // 运算符
    "{",
    "}",
    "[",
    "]",
    "(",
    ")",
    "+",
    "-",
    "*",
    "/",
    "%",
    "++",
    "--",
    "==",
    "!=",
    ">",
    "<",
    ">=",
    "<=",
    "&&",
    "||",
    "!",
    "&",
    "|",
    "~",
    "^",
    "<<",
    ">>",
    "=",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    "<<=",
    ">>=",
    "&=",
    "|=",
    "^=",
    ",",
    ";",
    // 关键字
    "auto",
    "break",
    "case",
    "char",
    "const",
    "continue",
    "default",
    "do",
    "double",
    "else",
    "enum",
    "extern",
    "float",
    "for",
    "goto",
    "if",
    "int",
    "long",
    "register",
    "return",
    "short",
    "signed",
    "sizeof",
    "static",
    "struct",
    "switch",
    "typedef",
    "union",
    "unsigned",
    "void",
    "volatile",
    "while",
    // 特殊字
    "Identifier", // 变量
    "Word"        // 无定义
};

// 非终结符列表 - 26
static const char *VNNAMES[]{
    "emptypro",
    "A",
    "P",
    "M",
    "N",
    "add_expression",
    "argument_list",
    "assign_sentence",
    "declare",
    "declare_list",
    "expression",
    "factor",
    "function_declare",
    "if_sentence",
    "inner_declare",
    "inner_var_declare",
    "item",
    "param",
    "parameter",
    "parameter_list",
    "return_sentence",
    "sentence",
    "sentence_block",
    "sentence_list",
    "var_declare",
    "while_sentence",
};

// 产生式文法数组
static const char *PATTERNS[] = {
    "P,N,declare_list",
    "declare_list,declare,declare_list",
    "declare_list,declare",
    "declare,'int','Identifier',M,A,function_declare",
    "declare,'int','Identifier',var_declare",
    //"declare,'int','Identifier',array_declare",           //数组定义
    "declare,'void','Identifier',M,A,function_declare",
    "A,emptypro",
    "var_declare,';'",
    "function_declare,'(',parameter,')',sentence_block",
    //"array_declare,'[',Integer,']'",                      //数组定义
    "parameter,parameter_list",
    "parameter,'void'",
    "parameter_list,param",
    "parameter_list,param,',',parameter_list",
    "param,'int','Identifier'",
    "sentence_block,'{',inner_declare,sentence_list,'}'",
    "inner_declare,emptypro",
    "inner_declare,inner_var_declare,';',inner_declare",
    "inner_var_declare,'int','Identifier'",
    "sentence_list,sentence,M,sentence_list",
    "sentence_list,sentence",
    "sentence,if_sentence",
    "sentence,while_sentence",
    "sentence,return_sentence",
    "sentence,assign_sentence",
    "assign_sentence,'Identifier','=',expression,';'",
    "return_sentence,'return',';'",
    "return_sentence,'return',expression,';'",
    "while_sentence,'while',M,'(',expression,')',A,sentence_block",
    "if_sentence,'if','(',expression,')',A,sentence_block",
    "if_sentence,'if','(',expression,')',A,sentence_block,N,'else',M,A,sentence_block",
    "N,emptypro",
    "M,emptypro",
    "expression,add_expression",
    "expression,add_expression,'>',add_expression",
    "expression,add_expression,'<',add_expression",
    "expression,add_expression,'==',add_expression",
    "expression,add_expression,'>=',add_expression",
    "expression,add_expression,'<=',add_expression",
    "expression,add_expression,'!=',add_expression",
    "add_expression,item",
    "add_expression,item,'+',add_expression",
    "add_expression,item,'-',add_expression",
    "item,factor",
    "item,factor,'*',item",
    "item,factor,'/',item",
    "factor,'Integer'",
    //"factor,'Floating'",                                  //浮点型数据
    "factor,'(',expression,')'",
    "factor,'Identifier','(',argument_list,')'",
    "factor,'Identifier'",
    "argument_list,emptypro",
    "argument_list,expression",
    "argument_list,expression,',',argument_list",
    "NULL"};

//-----------------------------------------------------------------------

#define START_STATE 0 // 初始态   -->  从1开始！！！
#define START_WORD 0  // 初始栈底 -->  '#'

#define RETURN_ERROR 0 // 错误返回
#define RETURN_FINE 1  // 正确返回

#define MAXN 1000          // 项目机组最大个数
#define TBSTATE_ACC -3     // 接受态
#define TBSTATE_REGULAR -2 // 规约
#define TBSTATE_SHIFT -1   // 移进
#define TBSTATE_NONE -4    // 空白错误态

#define NONIDLE -1 // 非待用

//-----------------------------------------------------------------------

//@func : 数据类型（int/void）
enum DType
{
    VOIDType,
    INTType
};

//-----------------------------------------------------------------------
//@func : 四元式结构体
struct Quaternary
{
    string op;   // 操作符
    string src1; // 源操作数1
    string src2; // 源操作数2
    string dst;  // 目的操作数
};
//@func : 基本快
struct Block
{
    string name;              // 基本块的名称
    vector<Quaternary> codes; // 基本块中的四元式
    int next1;                // 基本块的下一连接块
    int next2;                // 基本块的下一连接块
};
//@func : 基本功能块
struct FuncBlock
{
    string serial;        // 基本功能块编号
    string name;          // 基本功能块的名字
    vector<Block> blocks; // 基本功能块
};
//@func : 绘制树结构体
struct Tree
{
    int key;
    int value;
    vector<Tree *> children;
    Tree(int k, int v)
    {
        key = k;
        value = v;
    }
};
//@func : 从词法分析结果读取到的值
struct Token
{
    int id;
    string tokenType;
    string tokenValue;
};
//@func : 变量表结构体
struct Var
{
    string name;
    DType type;
    int level;
};
//@func : 函数表结构体
struct Func
{
    string name;
    DType returnType;
    list<DType> paramTypes;
    int enterPoint;
};
//@func : 项目集结构体 | eg : S -> .BB, #
struct Item // 单个项目
{
    int nump;       // 产生式编号
    int ppos;       //.的位置
    string forward; // 项目的向前搜索符集，比如 S -> .BB, a/b 。则 forward=“ab”.
    bool operator==(const Item &other) const
    {
        return nump == other.nump && ppos == other.ppos && forward == other.forward;
    }
};
//@func : 项目集族无向图边结构体
struct Edge
{
    int from;      // 以链表形式串联记录edge的所属
    int toItemSet; // 该条edge的指向的项目集
    int weight;    // 对应的V
};

//-----------------------------------------------------------------------

// 文件夹
const string folderPath = "./tmp/"; // 临时文件夹
const string srcPath = "./src/";    // 资源文件夹

// 资源文件
const string lr1Table = "lr1Table.txt";           // LR1表
const string lr1TableSR = "lr1TableSR.txt";       // LR1SR表
const string lr1TableShow = "lr1TableShow.txt";   // LR1表用于显示
const string lr1Production = "lr1Production.txt"; // LR1表达式

// 中间文件
const string input = "input.txt";           // 输入文件
const string lexOutput = "lexOutput.txt";   // 词法输出文件
const string lexResShow = "lexResShow.txt"; // 展示使用的词法分析结果
const string synProcess = "synProcess.txt"; // 语法分析移进规约过程
const string semCode = "semCode.txt";       // 语义分析结果
const string intCode = "interCode.txt";     // 中间代码[分块后]
const string objCode = "objCode.txt";       // 中间代码结果

const string synTree = "synTree.dot";  // 生成的树文件
const string synTreeImage = "synTree"; // 用于展示的图片

const string varFile = "varTable.txt";   // 代码生成的变量表
const string funcFile = "funcTable.txt"; // 代码生成的函数表

// 默认保存的文件名
const string defaultCodeSavePath = "./files/"; // 默认保存的文件目录
const string defaultCodeSaveFile = "unnamed";  // 默认保存code文件名

// const string defaultAnsSaveFile = "ans.txt";            //默认保存ans文件名

#endif // !_SOURCE_H
