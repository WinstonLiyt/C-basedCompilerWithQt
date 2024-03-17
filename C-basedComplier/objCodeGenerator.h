#pragma once

#ifndef _OBJCODEGENERATOR_H
#define _OBJCODEGENERATOR_H

//@func   :  目标代码生成器

#include <string>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <fstream>
#include <iostream>

#include "source.h"

// #include"interCodeGenerator.h"

using namespace std;

//-----------------------------------------------------------------------

//@func : 参数的待用活跃信息
class VariableInfo
{
public:
	int next;	   // 待用信息 -- 后续该块的第几个式子会使用到[num : 会 | ^ : 非待用]
	bool isActive; // 活跃信息 -- 后续是否还会使用[y : 会 | ^ : 不会]

	VariableInfo(int next, bool isActive);
	VariableInfo(const VariableInfo &other);
	VariableInfo() {}
	void print(ostream &fileout);
};
//@func : 带信息的四元式
class QuaternaryWithInfo
{
public:
	Quaternary quad;
	VariableInfo src1Info;
	VariableInfo src2Info;
	VariableInfo dstInfo;

	QuaternaryWithInfo(Quaternary quad, VariableInfo src1Info, VariableInfo src2Info, VariableInfo dstInfo) : quad(quad), src1Info(src1Info), src2Info(src2Info), dstInfo(dstInfo) {}

	void print(ostream &fileout);
};
//@func : 带信息的四元式基本块
struct InfoBlock
{
	string blockName;
	vector<QuaternaryWithInfo> instr;
	int nextBlock1;
	int nextBlock2;
};

//@func		: 生成目标代码
//@notice : 保存临时常数-t0 t1寄存器 | 保存函数的返回值-v0寄存器
class objectCodeGenerator
{
private:
	map<string, vector<InfoBlock>> funcInfoBlocks;

	map<string, set<string>> variableAddressDescriptor; // 变量的地址描述符
	map<string, set<string>> registerValueDescriptor;	// 寄存器的值描述符
	map<string, int> variableStorageOffset;				// 变量在存储器中的偏移位置
	map<string, string> functionSerialNames;			// 函数序列号与名称的映射
	int top;											// 当前栈顶位置
	list<string> availableRegisters;					// 可用的寄存器列表

	map<string, vector<set<string>>> exitLiveVariables;	 // 各函数块中基本块的出口活跃变量集
	map<string, vector<set<string>>> entryLiveVariables; // 各函数块中基本块的入口活跃变量集

	string currentFunction;									// 当前分析的函数编号
	vector<InfoBlock>::iterator currentInfoBlock;			// 当前分析的基本块
	vector<QuaternaryWithInfo>::iterator currentQuaternary; // 当前分析的四元式
	vector<string> generatedCode;							// 生成的目标代码

private:
	void storeVariable(string reg, string var);		// 将该数据存储到存储器中
	void storeExitLiveVariables(set<string> &outl); // 存储活跃变量至存储器中
	void releaseVariable(string var);				// 存储器中释放该数据

	string allocateRegisterForTarget();	 // 为目标变量分配寄存器
	string allocateRegister();			 // 为引用变量分配寄存器
	string allocateRegister(string var); // 为引用变量分配寄存器

	void generateFunBlocks(map<string, vector<InfoBlock>>::iterator &fiter);
	void generateBaseBlocks(int nowBaseBlockIndex);
	void generateQuaternaryCode(int nowBaseBlockIndex, int &arg_num, int &par_num, list<pair<string, bool>> &par_list);

public:
	objectCodeGenerator() {}

	void analyseFuncBlocks(vector<FuncBlock> *funcBlocks); //	根据block生成对应的待用活跃信息
	void generateCode();								   //	生成目标MIPS代码

	int refresh(void); //	更新数据

	// 结果输出
	void outputEntryExitLiveVariables(ostream &fileout); // not used
	void outputVariableOffsets(ostream &fileout);		 // not used
	void outputInfoBlocksDetails(ostream &fileout);
	void outputGeneratedCode(const char *fileName);
};

#endif
