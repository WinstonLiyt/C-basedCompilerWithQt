//@func   :  目标代码生成器代码

#include "objCodeGenerator.h"

bool isNum(string blockName)
{
	return isdigit(blockName[0]);
}

//@func :
VariableInfo::VariableInfo(int next, bool isActive)
{
	this->next = next;
	this->isActive = isActive;
}
//@func :
VariableInfo::VariableInfo(const VariableInfo &other)
{
	this->isActive = other.isActive;
	this->next = other.next;
}

//@func :
int objectCodeGenerator::refresh(void)
{
	this->funcInfoBlocks.clear();
	this->variableAddressDescriptor.clear();
	this->registerValueDescriptor.clear();
	this->variableStorageOffset.clear();
	this->functionSerialNames.clear();
	this->top = 0;
	this->availableRegisters.clear();
	this->exitLiveVariables.clear();
	this->entryLiveVariables.clear();
	this->currentFunction.clear();
	this->generatedCode.clear();

	return RETURN_FINE;
}

//@func : 存储变量
void objectCodeGenerator::storeVariable(string reg, string var)
{
	// 如果已经为*iter分配好了存储空间
	if (variableStorageOffset.find(var) != variableStorageOffset.end())
		generatedCode.push_back(string("sw ") + reg + " " + to_string(variableStorageOffset[var]) + "($sp)");
	// 如果暂未为*iter分配好了存储空间
	else
	{
		variableStorageOffset[var] = top;
		top += 4;
		generatedCode.push_back(string("sw ") + reg + " " + to_string(variableStorageOffset[var]) + "($sp)");
	}
	variableAddressDescriptor[var].insert(var);
}

//@func : 释放变量
void objectCodeGenerator::releaseVariable(string var)
{
	for (set<string>::iterator iter = variableAddressDescriptor[var].begin(); iter != variableAddressDescriptor[var].end(); iter++)
	{
		if ((*iter)[0] == '$')
		{
			registerValueDescriptor[*iter].erase(var);
			if (registerValueDescriptor[*iter].size() == 0 && (*iter)[1] == 's')
				availableRegisters.push_back(*iter);
		}
	}
	variableAddressDescriptor[var].clear();
}

//@func : 为引用变量分配寄存器
string objectCodeGenerator::allocateRegister()
{

	// 如果有尚未分配的寄存器，则从中选取一个Ri为所需要的寄存器R
	string ret;
	if (availableRegisters.size())
	{
		ret = availableRegisters.back();
		availableRegisters.pop_back();
		return ret;
	}

	// 如果没有尚未分配的寄存器，则从中选取一个Ri为所需要的寄存器R
	/*
	从已分配的寄存器中选取一个Ri为所需要的寄存器R。最好使得Ri满足以下条件：
		* 占用Ri的变量的值也同时存放在该变量的贮存单元中
		* 在基本块中要在最远的将来才会引用到或不会引用到
	*/

	const int inf = 1000000;
	int maxNextPos = 0;
	// 遍历所有的寄存器
	for (map<string, set<string>>::iterator iter = registerValueDescriptor.begin(); iter != registerValueDescriptor.end(); iter++)
	{
		int nextpos = inf;
		// 遍历寄存器中储存的变量
		for (set<string>::iterator viter = iter->second.begin(); viter != iter->second.end(); viter++)
		{
			bool inFlag = false; // 变量已在其他地方存储的标志
			// 遍历变量的存储位置
			for (set<string>::iterator aiter = variableAddressDescriptor[*viter].begin(); aiter != variableAddressDescriptor[*viter].end(); aiter++)
			{
				// 如果变量存储在其他地方
				if (*aiter != iter->first)
				{
					inFlag = true;
					break;
				}
			}
			// 如果变量仅存储在寄存器中，就看未来在何处会引用该变量
			if (!inFlag)
			{
				for (vector<QuaternaryWithInfo>::iterator cIter = currentQuaternary; cIter != currentInfoBlock->instr.end(); cIter++)
				{
					if (*viter == cIter->quad.src1 || *viter == cIter->quad.src2)
						nextpos = cIter - currentQuaternary;
					else if (*viter == cIter->quad.dst)
						break;
				}
			}
		}
		// 迭代
		if (nextpos == inf)
		{
			ret = iter->first;
			break;
		}
		else if (nextpos > maxNextPos)
		{
			maxNextPos = nextpos;
			ret = iter->first;
		}
	}

	for (set<string>::iterator iter = registerValueDescriptor[ret].begin(); iter != registerValueDescriptor[ret].end(); iter++)
	{
		// 对ret的寄存器中保存的变量*iter，他们都将不再存储在ret中
		variableAddressDescriptor[*iter].erase(ret);
		// 如果V的地址描述数组AVALUE[V]说V还保存在R之外的其他地方，则不需要生成存数指令
		if (variableAddressDescriptor[*iter].size() > 0)
		{
		}
		// 如果V不会在此之后被使用，则不需要生成存数指令
		else
		{
			bool storeFlag = true;
			vector<QuaternaryWithInfo>::iterator cIter;
			for (cIter = currentQuaternary; cIter != currentInfoBlock->instr.end(); cIter++)
			{
				// 如果V在本基本块中被引用
				if (cIter->quad.src1 == *iter || cIter->quad.src2 == *iter)
				{
					storeFlag = true;
					break;
				}
				// 如果V在本基本块中被赋值
				if (cIter->quad.dst == *iter)
				{
					storeFlag = false;
					break;
				}
			}
			// 如果V在本基本块中未被引用，且也没有被赋值
			if (cIter == currentInfoBlock->instr.end())
			{
				int index = currentInfoBlock - funcInfoBlocks[currentFunction].begin();
				// 如果此变量是出口之后的活跃变量
				if (exitLiveVariables[currentFunction][index].count(*iter) == 1)
					storeFlag = true;
				else
					storeFlag = false;
			}
			if (storeFlag) // 生成存数指令
				storeVariable(ret, *iter);
		}
	}
	registerValueDescriptor[ret].clear(); // 清空ret寄存器中保存的变量

	return ret;
}
//@func : 为引用变量分配寄存器
string objectCodeGenerator::allocateRegister(string var)
{

	// 变量是否为立即数
	if (isNum(var))
	{
		string ret = allocateRegister();
		generatedCode.push_back(string("addi ") + ret + " $zero " + var);
		return ret;
	}

	// 该变量是否在某个寄存器中
	for (set<string>::iterator iter = variableAddressDescriptor[var].begin(); iter != variableAddressDescriptor[var].end(); iter++)
		if ((*iter)[0] == '$') // 如果变量已经保存在某个寄存器中
			return *iter;	   // 直接返回该寄存器

	// 如果该变量没有在某个寄存器中
	string ret = allocateRegister();
	generatedCode.push_back(string("lw ") + ret + " " + to_string(variableStorageOffset[var]) + "($sp)"); // 从内存送入
	variableAddressDescriptor[var].insert(ret);
	registerValueDescriptor[ret].insert(var);
	return ret;
}

//@func : 为目标变量分配寄存器
string objectCodeGenerator::allocateRegisterForTarget()
{

	// i: A:=B op C
	// 如果B的现行值在某个寄存器Ri中，RVALUE[Ri]中只包含B
	// 此外，或者B与A是同一个标识符或者B的现行值在执行四元式A:=B op C之后不会再引用
	// 则选取Ri为所需要的寄存器R

	// 如果src1不是数字
	if (!isNum(currentQuaternary->quad.src1))
	{
		// 遍历src1所在的寄存器
		set<string> &src1pos = variableAddressDescriptor[currentQuaternary->quad.src1];
		for (set<string>::iterator iter = src1pos.begin(); iter != src1pos.end(); iter++)
		{
			if ((*iter)[0] == '$')
			{
				// 如果该寄存器中值仅仅存有src1
				if (registerValueDescriptor[*iter].size() == 1)
				{
					// 如果A,B是同一标识符或B以后不活跃
					if (currentQuaternary->quad.dst == currentQuaternary->quad.src1 || !currentQuaternary->src1Info.isActive)
					{
						variableAddressDescriptor[currentQuaternary->quad.dst].insert(*iter);
						registerValueDescriptor[*iter].insert(currentQuaternary->quad.dst);
						return *iter;
					}
				}
			}
		}
	}

	// 为目标变量分配可能不正确
	// return allocateRegister(currentQuaternary->quad.dst);
	string ret = allocateRegister();
	variableAddressDescriptor[currentQuaternary->quad.dst].insert(ret);
	registerValueDescriptor[ret].insert(currentQuaternary->quad.dst);
	return ret;
}

bool isVariable(string blockName)
{
	return isalpha(blockName[0]);
}

//@func :
void objectCodeGenerator::analyseFuncBlocks(vector<FuncBlock> *funcBlocks)
{
	// 遍历所有的函数块
	for (auto funBlockIter = funcBlocks->begin(); funBlockIter != funcBlocks->end(); funBlockIter++)
	{
		int blockIndex = 0;							  // 记录每个功能块的基本块编号
		vector<InfoBlock> infoBlocks;				  // 存储解析后的基本块信息
		vector<Block> &blocks = funBlockIter->blocks; // 引用当前函数块中的所有基本块
		vector<set<string>> INL, OUTL, DEF, USE;	  // 存储每个基本块的入口和出口活跃变量集合，以及定义和使用集合

		// ============= 确定funcOUTL and entryLiveVariables =============
		// 活跃变量的数据流方程
		// 确定每个基本块的DEF和USE集合
		for (vector<Block>::iterator blockIter = blocks.begin(); blockIter != blocks.end(); blockIter++)
		{
			set<string> def, use;
			for (vector<Quaternary>::iterator citer = blockIter->codes.begin(); citer != blockIter->codes.end(); citer++)
			{
				// 跳转和函数调用指令不直接参与变量的定义和使用
				if (citer->op == "j" || citer->op == "call")
				{
					;
				}
				// 处理条件跳转指令
				else if (citer->op[0] == 'j')
				{
					// 如果操作数未被定义，则视为使用
					if (isVariable(citer->src1) && def.count(citer->src1) == 0)
						use.insert(citer->src1);
					if (isVariable(citer->src2) && def.count(citer->src2) == 0)
						use.insert(citer->src2);
				}
				else
				{
					// 记录变量的使用和定义
					if (isVariable(citer->src1) && def.count(citer->src1) == 0)
						use.insert(citer->src1);
					if (isVariable(citer->src2) && def.count(citer->src2) == 0)
						use.insert(citer->src2);
					if (isVariable(citer->dst) && use.count(citer->dst) == 0)
						def.insert(citer->dst);
				}
			}
			// 记录基本块的入口活跃变量和定义变量
			INL.push_back(use);
			DEF.push_back(def);
			USE.push_back(use);
			OUTL.push_back(set<string>()); // 初始化输出活跃变量集合
		}

		// 活跃变量分析，计算INL和OUTL
		bool change = true;
		while (change)
		{
			change = false;
			blockIndex = 0;
			for (vector<Block>::iterator blockIter = blocks.begin(); blockIter != blocks.end(); blockIter++, blockIndex++)
			{
				int nextBlock1 = blockIter->next1;
				int nextBlock2 = blockIter->next2;
				// 更新当前基本块的OUTL集合
				if (nextBlock1 != -1)
				{
					for (set<string>::iterator inlIter = INL[nextBlock1].begin(); inlIter != INL[nextBlock1].end(); inlIter++)
					{
						// 下一块输入活跃项 = 该块的输出活跃项
						if (OUTL[blockIndex].insert(*inlIter).second == true)
						{
							if (DEF[blockIndex].count(*inlIter) == 0)
								INL[blockIndex].insert(*inlIter);
							change = true;
						}
					}
				}
				if (nextBlock2 != -1)
				{
					for (set<string>::iterator inlIter = INL[nextBlock2].begin(); inlIter != INL[nextBlock2].end(); inlIter++)
					{
						// 下一块输入活跃项 = 该块的输出活跃项
						if (OUTL[blockIndex].insert(*inlIter).second == true)
						{
							// 如果该块未定义项 = 为该块的输入活跃项
							if (DEF[blockIndex].count(*inlIter) == 0)
								INL[blockIndex].insert(*inlIter);
							change = true;
						}
					}
				}
			}
		}
		// 记录当前函数的出口和入口活跃变量信息
		exitLiveVariables[funBlockIter->serial] = OUTL;
		entryLiveVariables[funBlockIter->serial] = INL;

		// 构建信息块和符号表
		for (vector<Block>::iterator iter = blocks.begin(); iter != blocks.end(); iter++)
		{
			InfoBlock InfoBlock;
			InfoBlock.nextBlock1 = iter->next1;
			InfoBlock.nextBlock2 = iter->next2;
			InfoBlock.blockName = iter->name;
			for (vector<Quaternary>::iterator qIter = iter->codes.begin(); qIter != iter->codes.end(); qIter++)
				InfoBlock.instr.push_back(QuaternaryWithInfo(*qIter, VariableInfo(-1, false), VariableInfo(-1, false), VariableInfo(-1, false)));
			infoBlocks.push_back(InfoBlock);
		}

		vector<map<string, VariableInfo>> symTables; // 每个基本块的符号表
		for (vector<Block>::iterator blockIter = blocks.begin(); blockIter != blocks.end(); blockIter++)
		{
			map<string, VariableInfo> symTable;
			// 遍历基本块中的每个四元式
			for (vector<Quaternary>::iterator citer = blockIter->codes.begin(); citer != blockIter->codes.end(); citer++)
			{
				// pass
				if (citer->op == "j" || citer->op == "call")
				{
					;
				}
				// j>= j<=,j==,j!=,j>,j<
				else if (citer->op[0] == 'j')
				{
					if (isVariable(citer->src1))
						symTable[citer->src1] = VariableInfo{-1, false};
					if (isVariable(citer->src2))
						symTable[citer->src2] = VariableInfo{-1, false};
				}
				else
				{
					if (isVariable(citer->src1))
						symTable[citer->src1] = VariableInfo{-1, false};
					if (isVariable(citer->src2))
						symTable[citer->src2] = VariableInfo{-1, false};
					if (isVariable(citer->dst))
						symTable[citer->dst] = VariableInfo{-1, false};
				}
			}
			symTables.push_back(symTable);
		}

		// 更新基本块中变量的活跃信息
		blockIndex = 0;
		for (vector<set<string>>::iterator iter = OUTL.begin(); iter != OUTL.end(); iter++, blockIndex++)
			// 遍历活跃变量表中的变量
			for (set<string>::iterator viter = iter->begin(); viter != iter->end(); viter++)
				symTables[blockIndex][*viter] = VariableInfo{-1, true};

		// 计算每个四元式的待用信息和活跃信息
		blockIndex = 0;
		// 遍历每一个基本块
		for (vector<InfoBlock>::iterator ibiter = infoBlocks.begin(); ibiter != infoBlocks.end(); ibiter++, blockIndex++)
		{
			int codeIndex = ibiter->instr.size() - 1;
			// 逆序遍历基本块中的代码
			for (vector<QuaternaryWithInfo>::reverse_iterator citer = ibiter->instr.rbegin(); citer != ibiter->instr.rend(); citer++, codeIndex--)
			{
				if (citer->quad.op == "j" || citer->quad.op == "call")
				{
					;
				}
				else if (citer->quad.op[0] == 'j')
				{
					if (isVariable(citer->quad.src1))
					{
						citer->src1Info = symTables[blockIndex][citer->quad.src1];
						symTables[blockIndex][citer->quad.src1] = VariableInfo{codeIndex, true}; // update
					}
					if (isVariable(citer->quad.src2))
					{
						citer->src2Info = symTables[blockIndex][citer->quad.src2];
						symTables[blockIndex][citer->quad.src2] = VariableInfo{codeIndex, true}; // update
					}
				}
				else
				{
					if (isVariable(citer->quad.src1))
					{
						citer->src1Info = symTables[blockIndex][citer->quad.src1];
						symTables[blockIndex][citer->quad.src1] = VariableInfo{codeIndex, true}; // update
					}
					if (isVariable(citer->quad.src2))
					{
						citer->src2Info = symTables[blockIndex][citer->quad.src2];
						symTables[blockIndex][citer->quad.src2] = VariableInfo{codeIndex, true}; // update
					}
					if (isVariable(citer->quad.dst))
					{
						citer->dstInfo = symTables[blockIndex][citer->quad.dst];
						symTables[blockIndex][citer->quad.dst] = VariableInfo{-1, false}; // update
					}
				}
			}
		}
		// 将解析后的信息块信息保存到全局变量中
		funcInfoBlocks[funBlockIter->serial] = infoBlocks;
		functionSerialNames[funBlockIter->serial] = funBlockIter->name;
	}
}
//@func :
void objectCodeGenerator::storeExitLiveVariables(set<string> &outl)
{
	for (set<string>::iterator oiter = outl.begin(); oiter != outl.end(); oiter++)
	{
		string reg;			 //	活跃变量所在的寄存器名称
		bool inFlag = false; //	活跃变量在内存中的标志

		// 该活跃变量已经存储在内存中
		for (set<string>::iterator aiter = variableAddressDescriptor[*oiter].begin(); aiter != variableAddressDescriptor[*oiter].end(); aiter++)
		{
			if ((*aiter)[0] != '$')
			{
				inFlag = true;
				break;
			}
			else
			{
				reg = *aiter;
			}
		}
		// 如果该活跃变量不在内存中，则将reg中的var变量存入内存
		if (!inFlag)
		{
			storeVariable(reg, *oiter);
		}
	}
}
//@func :
void objectCodeGenerator::generateQuaternaryCode(int nowBaseBlockIndex, int &arg_num, int &par_num, list<pair<string, bool>> &par_list)
{
	// 检查当前四元式的操作数是否在使用前已经被定义
	if (currentQuaternary->quad.op[0] != 'j' && currentQuaternary->quad.op != "call")
	{
		if (isVariable(currentQuaternary->quad.src1) && variableAddressDescriptor[currentQuaternary->quad.src1].empty())
			Message += ">>> 【错误】变量" + currentQuaternary->quad.src1 + "在引用前未赋值\n";
		if (isVariable(currentQuaternary->quad.src2) && variableAddressDescriptor[currentQuaternary->quad.src2].empty())
			Message += ">>> 【错误】变量" + currentQuaternary->quad.src2 + "在引用前未赋值\n";
	}
	// 处理不同类型的四元式操作符
	// 如果是无条件跳转
	if (currentQuaternary->quad.op == "j")
		generatedCode.push_back("j " + currentQuaternary->quad.dst);
	// 如果是条件跳转
	else if (currentQuaternary->quad.op[0] == 'j')
	{
		string op;
		// 根据不同的条件跳转类型选择相应的MIPS操作码
		if (currentQuaternary->quad.op == "j>=")
			op = "bge";
		else if (currentQuaternary->quad.op == "j<=")
			op = "ble";
		else if (currentQuaternary->quad.op == "j==")
			op = "beq";
		else if (currentQuaternary->quad.op == "j!=")
			op = "bne";
		else if (currentQuaternary->quad.op == "j>")
			op = "bgt";
		else if (currentQuaternary->quad.op == "j<")
			op = "blt";

		// 为操作数分配寄存器
		string pos1 = allocateRegister(currentQuaternary->quad.src1);
		string pos2 = allocateRegister(currentQuaternary->quad.src2);
		generatedCode.push_back(op + " " + pos1 + " " + pos2 + " " + currentQuaternary->quad.dst);

		// 根据活跃信息判断是否需要释放操作数所在的寄存器
		if (!currentQuaternary->src1Info.isActive)
			releaseVariable(currentQuaternary->quad.src1);
		if (!currentQuaternary->src2Info.isActive)
			releaseVariable(currentQuaternary->quad.src2);
	}
	// 如果是函数参数传递，将参数添加到参数列表中
	else if (currentQuaternary->quad.op == "par")
		par_list.push_back(pair<string, bool>(currentQuaternary->quad.src1, currentQuaternary->src1Info.isActive));
	// 如果是函数调用，将参数压栈
	else if (currentQuaternary->quad.op == "call")
	{
		for (list<pair<string, bool>>::iterator aiter = par_list.begin(); aiter != par_list.end(); aiter++)
		{
			string pos = allocateRegister(aiter->first);
			generatedCode.push_back(string("sw ") + pos + " " + to_string(top + 4 * (++arg_num + 1)) + "($sp)");
			if (!aiter->second) // 如果参数在调用后不再活跃，则释放对应的寄存器
				releaseVariable(aiter->first);
		}
		// 更新栈顶指针
		generatedCode.push_back(string("sw $sp ") + to_string(top) + "($sp)");
		generatedCode.push_back(string("addi $sp $sp ") + to_string(top));

		// 生成跳转到函数的指令
		generatedCode.push_back(string("jal ") + currentQuaternary->quad.src1);

		// 恢复现场
		generatedCode.push_back(string("lw $sp 0($sp)"));
	}
	// 如果是函数返回
	else if (currentQuaternary->quad.op == "return")
	{
		// 返回值为数字
		if (isNum(currentQuaternary->quad.src1))
			generatedCode.push_back("addi $v0 $zero " + currentQuaternary->quad.src1);
		// 返回值为变量
		else if (isVariable(currentQuaternary->quad.src1))
		{
			set<string>::iterator piter = variableAddressDescriptor[currentQuaternary->quad.src1].begin();
			if ((*piter)[0] == '$') // 如果变量已在寄存器中
				generatedCode.push_back(string("add $v0 $zero ") + *piter);
			else
				generatedCode.push_back(string("lw $v0 ") + to_string(variableStorageOffset[*piter]) + "($sp)");
		}
		// 如果当前函数是main函数，则跳转到程序结束标签
		if (functionSerialNames[currentFunction] == "main")
			generatedCode.push_back("j end");
		// 返回到调用者
		else
		{
			generatedCode.push_back("lw $ra 4($sp)");
			generatedCode.push_back("jr $ra");
		}
	}
	// 如果是取地址操作
	else if (currentQuaternary->quad.op == "get")
	{
		variableStorageOffset[currentQuaternary->quad.dst] = top;
		top += 4;
		variableAddressDescriptor[currentQuaternary->quad.dst].insert(currentQuaternary->quad.dst);
	}
	// 如果是赋值操作
	else if (currentQuaternary->quad.op == "=")
	{
		string src1Pos;
		// 如果是函数返回值
		if (currentQuaternary->quad.src1 == "@RETURN_PLACE")
			src1Pos = "$v0";
		// 为源操作数分配寄存器
		else
			src1Pos = allocateRegister(currentQuaternary->quad.src1);
		// 更新寄存器和变量的描述符
		registerValueDescriptor[src1Pos].insert(currentQuaternary->quad.dst);
		variableAddressDescriptor[currentQuaternary->quad.dst].insert(src1Pos);
	}
	// 处理算术运算
	else
	{
		string src1Pos = allocateRegister(currentQuaternary->quad.src1);
		string src2Pos = allocateRegister(currentQuaternary->quad.src2);
		string desPos = allocateRegisterForTarget();
		if (currentQuaternary->quad.op == "+")
			generatedCode.push_back(string("add ") + desPos + " " + src1Pos + " " + src2Pos);
		else if (currentQuaternary->quad.op == "-")
			generatedCode.push_back(string("sub ") + desPos + " " + src1Pos + " " + src2Pos);
		else if (currentQuaternary->quad.op == "*")
			generatedCode.push_back(string("mul ") + desPos + " " + src1Pos + " " + src2Pos);
		else if (currentQuaternary->quad.op == "/")
		{
			generatedCode.push_back(string("div ") + src1Pos + " " + src2Pos);
			generatedCode.push_back(string("mflo ") + desPos);
		}
		// 根据操作数的活跃信息决定是否释放寄存器
		if (!currentQuaternary->src1Info.isActive)
			releaseVariable(currentQuaternary->quad.src1);
		if (!currentQuaternary->src2Info.isActive)
			releaseVariable(currentQuaternary->quad.src2);
	}
}

//@func :
bool isControlOP(string op)
{
	if (op[0] == 'j' || op == "call" || op == "return" || op == "get")
		return true;
	return false;
}

//@func :生成一个基本块内的目标代码。基本块是一段没有分支（除了入口和出口）的代码序列。
void objectCodeGenerator::generateBaseBlocks(int nowBaseBlockIndex)
{

	int arg_num = 0;				   // 函数调用的实参数量
	int par_num = 0;				   // 函数的形参数量
	list<pair<string, bool>> par_list; // 用于记录函数调用时的参数和它们是否活跃的列表

	// 清空变量地址描述和寄存器值描述，为当前基本块的代码生成做准备
	variableAddressDescriptor.clear();
	registerValueDescriptor.clear();

	// 根据当前基本块的入口活跃变量集初始化变量地址描述
	set<string> &inl = entryLiveVariables[currentFunction][nowBaseBlockIndex];
	for (auto iter = inl.begin(); iter != inl.end(); iter++)
		variableAddressDescriptor[*iter].insert(*iter);

	// 初始化空闲寄存器列表
	availableRegisters.clear();
	for (int i = 0; i <= 7; i++)
		availableRegisters.push_back("$s" + to_string(i));

	// 如果是基本块的第一条语句，需特殊处理（比如函数入口的处理）
	generatedCode.push_back(currentInfoBlock->blockName + ":");
	if (nowBaseBlockIndex == 0)
	{
		// main函数无需压栈
		if (functionSerialNames[currentFunction] == "main")
			top = 8;
		// 把返回地址压栈
		else
			generatedCode.push_back("sw $ra 4($sp)");
		top = 8;
	}

	// 遍历基本块内的每条四元式，生成目标代码
	for (vector<QuaternaryWithInfo>::iterator cIter = currentInfoBlock->instr.begin(); cIter != currentInfoBlock->instr.end(); cIter++)
	{
		currentQuaternary = cIter;
		// 处理基本块的最后一条语句，可能需要保存出口活跃变量
		if (cIter + 1 == currentInfoBlock->instr.end())
		{
			// 如果最后一条语句是控制语句，则先将出口活跃变量保存，再进行跳转（j，call，return)）
			if (isControlOP(cIter->quad.op))
			{
				storeExitLiveVariables(exitLiveVariables[currentFunction][nowBaseBlockIndex]);
				generateQuaternaryCode(nowBaseBlockIndex, arg_num, par_num, par_list);
			}
			// 如果最后一条语句不是控制语句（是赋值语句），则先计算，再将出口活跃变量保存
			else
			{
				generateQuaternaryCode(nowBaseBlockIndex, arg_num, par_num, par_list);
				storeExitLiveVariables(exitLiveVariables[currentFunction][nowBaseBlockIndex]);
			}
		}
		else
			generateQuaternaryCode(nowBaseBlockIndex, arg_num, par_num, par_list);
	}
}
//@func :遍历一个函数中的所有基本块
void objectCodeGenerator::generateFunBlocks(map<string, vector<InfoBlock>>::iterator &fiter)
{

	variableStorageOffset.clear();				   // 清空变量存储偏移信息
	currentFunction = fiter->first;				   // 设置当前正在处理的函数
	vector<InfoBlock> &InfoBlocks = fiter->second; // 获取当前函数的所有基本块信息

	// 遍历并生成每个基本块的目标代码
	for (vector<InfoBlock>::iterator iter = InfoBlocks.begin(); iter != InfoBlocks.end(); iter++)
	{
		currentInfoBlock = iter;
		generateBaseBlocks(currentInfoBlock - InfoBlocks.begin());
	}
}
//@func :初始化代码生成所需的数据结构
void objectCodeGenerator::generateCode()
{
	generatedCode.push_back("lui $sp,0x1001"); // 初始化栈指针
	generatedCode.push_back("j main");		   // 跳转到main函数开始执行

	// 遍历所有函数，生成它们的目标代码
	for (auto fiter = funcInfoBlocks.begin(); fiter != funcInfoBlocks.end(); fiter++)
		generateFunBlocks(fiter);

	generatedCode.push_back("end:");
}

//@func :生成目标代码
void objectCodeGenerator::outputGeneratedCode(const char *fileName)
{
	ofstream fout(fileName);
	if (!fout.is_open())
	{
		cerr << "【错误】文件 " << fileName << "无法打开！" << endl;
		Message += ">>> 【错误】文件" + string(fileName) + "无法打开！\n";
	}
	else
	{
		for (const auto &line : generatedCode)
			fout << line << endl;
		fout.close();
	}
}

//@func :
void VariableInfo::print(ostream &fileout)
{
	fileout << "(";
	if (next == NONIDLE)
		fileout << "^";
	else
		fileout << next;
	fileout << ",";
	if (isActive)
		fileout << "y";
	else
		fileout << "^";

	fileout << ")";
}

//@func :
void QuaternaryWithInfo::print(ostream &fileout)
{
	fileout << "(" << quad.op << "," << quad.src1 << "," << quad.src2 << "," << quad.dst << ")";
	src1Info.print(fileout);
	src2Info.print(fileout);
	dstInfo.print(fileout);
}

//@func :生成待用活-跃信息表
void objectCodeGenerator::outputInfoBlocksDetails(ostream &fileout)
{
	for (map<string, vector<InfoBlock>>::iterator iter = funcInfoBlocks.begin(); iter != funcInfoBlocks.end(); iter++)
	{
		fileout << "[" << iter->first << "]" << endl;
		for (vector<InfoBlock>::iterator bIter = iter->second.begin(); bIter != iter->second.end(); bIter++)
		{
			fileout << bIter->blockName << ":" << endl;
			for (vector<QuaternaryWithInfo>::iterator cIter = bIter->instr.begin(); cIter != bIter->instr.end(); cIter++)
			{
				fileout << "(" << cIter->quad.op << "," << cIter->quad.src1 << "," << cIter->quad.src2 << "," << cIter->quad.dst << ")";
				fileout << " ";
				cIter->dstInfo.print(fileout);
				fileout << " ";
				cIter->src1Info.print(fileout);
				fileout << " ";
				cIter->src2Info.print(fileout);
				fileout << endl;
			}
		}
	}
}

//@func :没用到
void objectCodeGenerator::outputEntryExitLiveVariables(ostream &fileout)
{
	for (auto iterOut = this->exitLiveVariables.begin(), iterIn = this->entryLiveVariables.begin(); (iterOut != this->exitLiveVariables.end()) && (iterIn != this->entryLiveVariables.end()); ++iterIn, ++iterOut)
	{
		fileout << "[function]" << iterOut->first << "\n";
		// 遍历活跃变量表中的变量
		int blockIndex = 0;
		for (auto BiterOut = iterOut->second.begin(), BiterIn = iterIn->second.begin(); (BiterOut != iterOut->second.end()) && (BiterIn != iterIn->second.end()); ++BiterIn, ++BiterOut, ++blockIndex)
		{
			fileout << "[block]" << blockIndex << "\n";
			// In
			fileout << "		"
					<< "IN  : ";
			for (auto IiterIn = BiterIn->begin(); (IiterIn != BiterIn->end()); ++IiterIn)
			{
				fileout << *IiterIn << " ";
			}
			fileout << "\n";
			// Out
			fileout << "		"
					<< "OUT : ";
			for (auto IiterOut = BiterOut->begin(); (IiterOut != BiterOut->end()); ++IiterOut)
			{
				fileout << *IiterOut << " ";
			}
			fileout << "\n";
		}
	}
}
//@func :没用到
void objectCodeGenerator::outputVariableOffsets(ostream &fileout)
{
	for (auto iter = this->variableStorageOffset.begin(); iter != this->variableStorageOffset.end(); ++iter)
	{
		fileout << iter->first << " " << iter->second << endl;
	}
	return;
}
