#pragma once
#ifndef _SYNTAXANAYZER_H
#define _SYNTAXANAYZER_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_set>
#include <queue>
#include <vector>
#include <stack>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "source.h"

using namespace std;

//@func :
class syntaxAnalyzer
{
private:
    vector<vector<int>> production;       // 转化后的产生式
    vector<string> getProduce;            // 每个V对应的产生式编号
    vector<string> first;                 // Fist集
    vector<vector<Item>> itemSet;         // 项目集族
    vector<vector<int>> tableLR1;         // LR1分析表
    vector<vector<int>> shiftReduceTable; // LR1分析表[用于判别是移进还是规约]

    vector<Edge> edges; // 边
    vector<int> head;   // 第i个项目集的头

public:
    map<string, int> symbolMap; // 非终结&终结符转化，1-85为终结符，86-112为非终结符
    vector<string> symbolStr;   // 编号-<str
    int numVt;                  // 终结符个数     85+1=86
    int numVn;                  // 非终结符个数   26
    int epsilon;                // ε对应的值      86[即非终结符中第一个]
    int productionNum;          // 产生式个数

public:
    void itemSetAddEgde(int from, int to, int w);                     // 添加边
    inline bool itemSetIfInSet(const Item &a, const vector<Item> &b); //
    vector<Item> itemSetMerge(vector<Item> a, vector<Item> b);        // 合并项目集 a,b 复给 a
    int itemSetSearch(vector<Item> &a, vector<vector<Item>> &b);      // 查找项目集，若有，则返回编号,一举俩得
    void printItemSetDetails() const;

    void initAnalyzer();
    vector<int> initProduc(const char *_pattern);
    void getFirstSet();
    void dfs(int nv, int nump, vector<bool> &getted);
    vector<Item> getItemClosure(Item item);
    void getItemSet();
    bool genLR1Table();

    void printLR1Table(const char *filename = (srcPath + lr1TableShow).c_str());     //  打印LR1分析表
    void genLR1Table_2(const char *filename = (srcPath + lr1Table).c_str());         //  将LR1表的信息存储到文件中
    void genLR1Production(const char *filename = (srcPath + lr1Production).c_str()); //  将产生式存储到文件中
    void genSRTable(const char *filename = (srcPath + lr1TableSR).c_str());          //  将SR表存储到文件中
    void genActionGOTOTable(const char *filename);
    void genActionGOTOTableCSV(const char *filename);

    vector<vector<int>> load_s_r_Table(const char *filename = (srcPath + lr1TableSR).c_str());        //  从文件中读取SR表的信息
    vector<vector<int>> load_LR1Production(const char *filename = (srcPath + lr1Production).c_str()); //  从文件中读取产生式的信息
    vector<vector<int>> load_LR1Table(const char *filename = (srcPath + lr1Table).c_str());           //  从文件中读取LR1表的信息

    int syntaxAnalyzeLR1Table(const char *LR1show = (srcPath + lr1TableShow).c_str(),
                              const char *LR1 = (srcPath + lr1Table).c_str(),
                              const char *LR1SR = (srcPath + lr1TableSR).c_str(),
                              const char *LR1Produncton = (srcPath + lr1Production).c_str());
};

#endif
