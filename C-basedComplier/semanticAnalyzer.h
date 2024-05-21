#pragma once

#ifndef _SEMANALYZER_H
#define _SEMANALYZER_H

#include <string>
#include <list>

#include "source.h"

using namespace std;

class Symbol
{
public:
	bool isVt;
	string val;
	friend bool operator==(const Symbol &x, const Symbol &y);
	friend bool operator<(const Symbol &x, const Symbol &y);
	Symbol(const Symbol &sym);
	Symbol(const bool &isVt, const string &val);
	Symbol();
};

class Identifier : public Symbol
{
public:
	string name;
	Identifier(const Symbol &sym, const string &name);
};

class Num : public Symbol
{
public:
	string number;
	Num(const Symbol &sym, const string &number);
};

class FunctionDeclaration : public Symbol
{
public:
	list<DType> parameterTypes;
	FunctionDeclaration(const Symbol &sym) : Symbol(sym){};
};

class Parameter : public Symbol
{
public:
	list<DType> parameterTypes;
	Parameter(const Symbol &sym) : Symbol(sym){};
};

class ParameterList : public Symbol
{
public:
	list<DType> parameterTypes;
	ParameterList(const Symbol &sym) : Symbol(sym){};
};

class StatementBlock : public Symbol
{
public:
	list<int> nextList;
	StatementBlock(const Symbol &sym) : Symbol(sym){};
};

class StatementList : public Symbol
{
public:
	list<int> nextList;
	StatementList(const Symbol &sym) : Symbol(sym){};
};

class Sentence : public Symbol
{
public:
	list<int> nextList;
	Sentence(const Symbol &sym) : Symbol(sym){};
};

class WhileSentence : public Symbol
{
public:
	list<int> nextList;
	WhileSentence(const Symbol &sym) : Symbol(sym){};
};

class IfSentence : public Symbol
{
public:
	list<int> nextList;
	IfSentence(const Symbol &sym) : Symbol(sym){};
};

class Expression : public Symbol
{
public:
	string name;
	list<int> falseList;
	Expression(const Symbol &sym) : Symbol(sym){};
};

class M : public Symbol
{
public:
	int quad;
	M(const Symbol &sym) : Symbol(sym){};
};

class N : public Symbol
{
public:
	list<int> nextList;
	N(const Symbol &sym) : Symbol(sym){};
};

class AddExpression : public Symbol
{
public:
	string name;
	AddExpression(const Symbol &sym) : Symbol(sym){};
};

class Nomial : public Symbol
{
public:
	string name;
	Nomial(const Symbol &sym) : Symbol(sym){};
};

class Factor : public Symbol
{
public:
	string name;
	Factor(const Symbol &sym) : Symbol(sym){};
};

class ArgumentList : public Symbol
{
public:
	list<string> alist;
	ArgumentList(const Symbol &sym) : Symbol(sym){};
};

#endif
