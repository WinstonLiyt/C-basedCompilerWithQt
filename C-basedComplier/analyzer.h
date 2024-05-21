#pragma once
#ifndef _ANALYZER_H
#define _ANALYZER_H

#include "lexialAnalyzer.h"
#include "syntaxAnalyzer.h"
#include "semanticAnalyzer.h"
#include "interCodeGenerator.h"
#include "objCodeGenerator.h"

class Compile_Analyzer
{

public:
	Compile_Analyzer();

private:
	// 根据文法而变化
	vector<vector<int>> production;		  // 转化后的产生式
	vector<vector<int>> tableLR1;		  // LR1分析表
	vector<vector<int>> shiftReduceTable; // LR1分析表[用于判别是移进还是规约]
	map<string, int> symbolMap;			  // 非终结&终结符转化,1-85为终结符，86-112为非终结符，舍弃旧字符，这里跟原来很不一样
	vector<string> symbolStr;			  // 编号->str
	int numVt;							  // 终结符个数     85+1=86
	int numVn;							  // 非终结符个数   26
	int epsilon;						  // ε对应的值     86[即非终结符中第一个]
	int productionNum;					  // 产生式个数

	// 随输入而变
	vector<Token> tokens;		   // 输入的终结符序列
	vector<Var> varTable;		   // 变量表
	vector<Func> funcTable;		   // 函数表
	stack<int> stateStack;		   // 状态栈
	stack<Symbol *> symbolStack;   // 符号栈
	LabelGenerator labelGenerator; // 生成新名

public:
	lexialAnalyzer lexicalAnalyzer;				  // 词法分析器
	syntaxAnalyzer syntaxAnalyzer;				  // 语法分析器
	IntermediateCodeGenerator interCodeGenerator; // 中间代码生成器
	objectCodeGenerator objectCodeGenerator;	  // 目标代码生成器

public:
	// 绘制树函数
	void generateSubtreeShape(ofstream &outputStream, Tree *&node);
	void generateSubtreeConnections(ofstream &outputStream, Tree *&node);
	void generateSyntaxTree(stack<vector<int>> &parseStack, const char *outputFilePath); // cmd打印树

	// 语义分析函数
	void outputStack(ostream &fileout); // 输出状态栈和符号栈
	Symbol *popSymbol();
	void pushSymbol(Symbol *sym);
	Func *searchFunction(string ID);
	Var *searchVariable(string ID);
	vector<Func> getFunTable(void);

	Symbol *createNextSymbol(const string &tokenType, const string &tokenValue);

	int get_words(const char *lexInput = (folderPath + lexOutput).c_str());
	int get_value_from_syn(void);
	int get_LR1_tables(const char *fileProduction = (srcPath + lr1Production).c_str(),
					   const char *fileTableLR1 = (srcPath + lr1Table).c_str(),
					   const char *fileTableSR = (srcPath + lr1TableSR).c_str());
	int regular_LR1(const char *AnalyzeProcess = (folderPath + synProcess).c_str(),
					const char *SynTaxTree = (folderPath + synTree).c_str());

	int initForAll(void);	 // 文法生成LR1
	int refresh(void);		 // 清除原有数据
	int lexAnalyzer(void);	 // 词法分析
	int synAnalyzer(void);	 // 语法分析
	int semAnalyzer(void);	 // 语义分析
	int interAnalyzer(void); // 中间代码
	int objAnalyzer(void);	 // 目标代码
};

#endif
