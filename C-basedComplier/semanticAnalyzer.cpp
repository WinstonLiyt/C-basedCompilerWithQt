#include "semanticAnalyzer.h"

bool operator==(const Symbol &x, const Symbol &y)
{
	return x.val == y.val;
}
bool operator<(const Symbol &x, const Symbol &y)
{
	return x.val < y.val;
}

Symbol::Symbol(const Symbol &sym)
{
	this->val = sym.val;
	this->isVt = sym.isVt;
}

Symbol::Symbol(const bool &isVt, const string &val)
{
	this->isVt = isVt;
	this->val = val;
}

Symbol::Symbol() {}

Identifier::Identifier(const Symbol &sym, const string &name) : Symbol(sym)
{
	this->name = name;
}

Num::Num(const Symbol &sym, const string &number) : Symbol(sym)
{
	this->number = number;
}
