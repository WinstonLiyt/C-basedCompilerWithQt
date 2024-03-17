#pragma once

#ifndef _INTERCODEGENERATOR_H
#define _INTERCODEGENERATOR_H

//@func   :  中间代码生成头文件

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

//@func : 生成四元式的Label
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

//@func :
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
    // vector<Quaternary> GetQuaternaries(void);

    void emit_1(Quaternary q);                                  // 生成四元式
    void Emit(string op, string src1, string src2, string dst); // 生成四元式
    void BackPatch(list<int> nextList, int quad);               // 代码回填

    void DivideIntoFunctionBlocks(vector<Func> funcTable); // 划分功能块
    vector<FuncBlock> *GetFunctionBlocks(void);            // 返回功能块

    void Output();                     // 输出
    void Output(ostream &out);         // 输出
    void Output(const char *fileName); // 输出

    void OutputFunctionBlocks(void);                 // 输出
    void OutputFunctionBlocks(ostream &out);         // 输出
    void OutputFunctionBlocks(const char *fileName); // 输出

    int refresh(void);
};

#endif
