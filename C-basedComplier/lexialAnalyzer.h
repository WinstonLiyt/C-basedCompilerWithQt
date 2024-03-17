#pragma once
#ifndef _LEXIALANAYZER_H
#define _LEXIALANAYZER_H

//@time   :  2022.10.7
//@func   :  词法分析器头文件

#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <assert.h>
#include <algorithm>
#include <map>
#include "source.h"

using namespace std;

/* 词法token切割 */
struct lexWord
{
    string lex;
    int start, end; // 文件中的位置(不包含end)
    int lexType;    // 类型

    lexWord(string lex, int start, int end, int lexType) : lex(lex), start(start), end(end), lexType(lexType) {}
};
bool cmp(lexWord l, lexWord r);

/* 字典树存储word */
class PrefixTree
{
private:
    PrefixTree *children[128]; // 节点的每个字符指向的下一个节点
    int wordType;              // 节点的类型，用于区分不同的词或其他用途
    bool isEndOfWord;          // 标记是否为某个单词的结束

public:
    PrefixTree();                                     // 构造函数
    void insert(const string &word, const int &type); // 插入单词和类型
    vector<lexWord> searchWords(const string &word);  // 根据前缀匹配单词
};

//------------------------------------ 词法分析类 ------------------------------------
class lexialAnalyzer
{
private:
    string sourceCode = "";    // 源代码内容
    int sourceCodeLength = 0;  // 源代码长度
    int *charTypes = NULL;     // 文件中的每一个字符对应的类型
    vector<lexWord> fragments; // 分析之后得到的每个片段
    PrefixTree trie;           // 字典树方便查找

    void preProcess(); // 处理宏定义、注释,字符，字符串，分隔符
    /* 词法判别（bool，int，float，标识符 */
    bool isBoolean(const string &str);
    bool isInt(const string &str);
    bool isFloat(const string &str);
    bool isIdentifier(const string &str);
    void spiltType(string str, int start, int end, int type); // 存储分析结果
    int stLexAns(ofstream &outFile, ofstream &outFile2, string buffer, vector<lexWord> &res);

    //======== 词法分析函数 ========
public:
    void lexialAnalyzeToken(const string &code); // 总的分析词法函数                                                     // 获取词法分析结果
    int lexialAnalyze(const char *inPath = (folderPath + input).c_str(),
                      const char *outPath = (folderPath + lexResShow).c_str(),
                      const char *outPath2 = (folderPath + lexOutput).c_str()); // 词法分析函数

    int initialLexicalAnalysis(); // lex初始化
    int refresh(void);            // lex更新
};
#endif
