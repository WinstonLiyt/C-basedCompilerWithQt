#pragma once

#ifndef _INTERCODEGENERATOR_H
#define _INTERCODEGENERATOR_H

#include <cstring>
#include <list>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <queue>
#include <map>
#include <string>

#include "source.h"

using namespace std;

// 生成四元式的Label
class LabelGenerator
{
private:
    int current_index;
    int starting_point;
    string base_label;

public:
    LabelGenerator();
    LabelGenerator(const char *base_label);
    LabelGenerator(int starting_point, const char *base_label);
    string new_label();
    int refresh();
};

class IntermediateCodeGenerator
{
public:
    IntermediateCodeGenerator();

    vector<string> errorMessage; // 存储错误提示

private:
    vector<Quaternary> quaternaries;        // 四元式
    LabelGenerator blockLabelGenerator;     // 基本块标记
    LabelGenerator blockFuncLabelGenerator; // 功能块标记
    vector<FuncBlock> blockFunc;            // 功能块
    void GenerateDotGraphsForAllFuncBlocks();

public:
    void Initialize();
    int NextQuaternaryIndex();
    void emit_1(Quaternary q);
    void Emit(string op, string src1, string src2, string dst); // 生成四元式
    void BackPatch(list<int> nextList, int quad);               // 代码回填

    void DivideIntoFunctionBlocks(vector<Func> funcTable); // 划分功能块
    vector<FuncBlock> *GetFunctionBlocks(void);            // 返回功能块

    void Output();
    void Output(ostream &out);
    void Output(const char *fileName);

    void OutputFunctionBlocks(void);
    void OutputFunctionBlocks(ostream &out);
    void OutputFunctionBlocks(const char *fileName);

    int refresh(void);
};

#endif
