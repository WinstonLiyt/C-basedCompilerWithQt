//@func   :  中间代码生成器

#include "interCodeGenerator.h"
#include "source.h"

//-----------------------------------------------------------------------

//@func :
LabelGenerator::LabelGenerator()
{
	this->current_index = 0;
	this->starting_point = 0;
	this->base_label = "label";
}
//@func :
LabelGenerator::LabelGenerator(const char *base_label)
{
	this->current_index = 0;
	this->starting_point = 0;
	this->base_label = base_label;
}
//@func :
LabelGenerator::LabelGenerator(int index, const char *base_label)
{
	this->current_index = index;
	this->starting_point = index;
	this->base_label = base_label;
}
//@func :
string LabelGenerator::new_label()
{
	return this->base_label + to_string(starting_point++);
}
//@func :
int LabelGenerator::refresh()
{
	this->starting_point = this->current_index;
	return RETURN_FINE;
}

//-----------------------------------------------------------------------

//@func :
IntermediateCodeGenerator::IntermediateCodeGenerator()
{
	this->blockLabelGenerator = LabelGenerator(0, "B");
	this->blockFuncLabelGenerator = LabelGenerator(0, "F");
}

//@func :
int IntermediateCodeGenerator::refresh()
{
	this->errorMessage.clear();
	this->quaternaries.clear();
	this->blockFunc.clear();
	this->blockLabelGenerator.refresh();
	this->blockFuncLabelGenerator.refresh();

	return RETURN_FINE;
}

//-----------------------------------------------------------------------

//@func :
void IntermediateCodeGenerator::emit_1(Quaternary q)
{
	quaternaries.push_back(q);
}
//@func :
void IntermediateCodeGenerator::Emit(string op, string src1, string src2, string dst)
{
	emit_1(Quaternary{op, src1, src2, dst});
}
//@func :
void IntermediateCodeGenerator::BackPatch(list<int> nextList, int quad)
{
	for (list<int>::iterator iter = nextList.begin(); iter != nextList.end(); iter++)
	{
		quaternaries[*iter].dst = to_string(quad);
	}
}

//-----------------------------------------------------------------------

//@func :
int IntermediateCodeGenerator::NextQuaternaryIndex()
{
	return quaternaries.size();
}
//@func :
void IntermediateCodeGenerator::Output(ostream &out)
{

	int i = 0;
	for (vector<Quaternary>::iterator iter = quaternaries.begin(); iter != quaternaries.end(); iter++, i++)
	{
		out << setw(4) << i;
		out << "( " << iter->op << " , ";
		out << iter->src1 << " , ";
		out << iter->src2 << " , ";
		out << iter->dst << " )";
		out << endl;
	}
}
//@func :
void IntermediateCodeGenerator::Output()
{
	Output(cout);
}
//@func :
void IntermediateCodeGenerator::Output(const char *fileName)
{
	ofstream fout;
	fout.open(fileName);
	if (!fout.is_open())
	{
		cerr << "file " << fileName << " open error" << endl;
		Message += "[error] can not open file";
		Message += fileName;
		Message += "\n";
		return;
	}
	Output(fout);
	fout.close();
}
//@func :
void IntermediateCodeGenerator::OutputFunctionBlocks(ostream &out)
{

	for (auto iter = blockFunc.begin(); iter != blockFunc.end(); iter++)
	{
		out << "[" << iter->name << "]" << endl;
		for (auto bIter = iter->blocks.begin(); bIter != iter->blocks.end(); bIter++)
		{
			out << bIter->name << ":" << endl;
			for (vector<Quaternary>::iterator cIter = bIter->codes.begin(); cIter != bIter->codes.end(); cIter++)
			{

				out << "    "
					<< "(" << cIter->op << "," << cIter->src1 << "," << cIter->src2 << "," << cIter->dst << ")" << endl;
			}
			out << "    "
				<< "next1 = " << bIter->next1 << endl;
			out << "    "
				<< "next2 = " << bIter->next2 << endl;
		}
		out << endl;
	}
}
//@func :
void IntermediateCodeGenerator::OutputFunctionBlocks()
{
	OutputFunctionBlocks(cout);
}
//@func :
void IntermediateCodeGenerator::OutputFunctionBlocks(const char *fileName)
{
	ofstream fout;
	fout.open(fileName);
	if (!fout.is_open())
	{
		cerr << "file " << fileName << " open error" << endl;
		Message += "[error] can not open file";
		Message += fileName;
		Message += "\n";
		return;
	}
	OutputFunctionBlocks(fout);

	fout.close();
}
//@func :
vector<FuncBlock> *IntermediateCodeGenerator::GetFunctionBlocks()
{
	return &blockFunc;
}

void IntermediateCodeGenerator::GenerateDotGraphsForAllFuncBlocks() {
    string dotFilename = (srcPath + "AllFuncBlocks.dot").c_str();  // 所有函数块的 .dot 文件名
    string svgFilename = (srcPath + "AllFuncBlocks.svg").c_str();  // 目标 SVG 文件名
    ofstream dotFile(dotFilename);

    dotFile << "digraph AllFuncBlocks {\n";
    dotFile << "    node [shape=record];\n";

    // 遍历所有的函数块
    for (const auto& funcBlock : this->blockFunc) {
        // 遍历函数块中的所有基本块
        for (size_t i = 0; i < funcBlock.blocks.size(); ++i) {
            const auto& block = funcBlock.blocks[i];
            dotFile << "    " << funcBlock.serial << "_" << block.name << " [label=\"{" << block.name << "|";

            // 添加每个基本块中的四元式代码
            for (const auto& code : block.codes)
                dotFile << code.op << ", " << code.src1 << ", " << code.src2 << ", " << code.dst << "\\l";
            dotFile << "}\"];\n";

            // 绘制转移
            if (block.next1 != -1 && (size_t)block.next1 < funcBlock.blocks.size())
                dotFile << "    " << funcBlock.serial << "_" << block.name << " -> " << funcBlock.serial << "_" << funcBlock.blocks[block.next1].name << ";\n";
            if (block.next2 != -1 && (size_t)block.next2 < funcBlock.blocks.size())
                dotFile << "    " << funcBlock.serial << "_" << block.name << " -> " << funcBlock.serial << "_" << funcBlock.blocks[block.next2].name << ";\n";
        }
    }

    dotFile << "}\n";
    dotFile.close();

    // 调用Graphviz的dot命令生成SVG文件
    string command = "dot -Tsvg " + dotFilename + " -o " + svgFilename;
    std::system(command.c_str());
}


//@func : 功能块划分
void IntermediateCodeGenerator::DivideIntoFunctionBlocks(vector<Func> funcTable)
{

	/* 遍历函数表，为每个函数生成基本块 */
	for (auto iter = funcTable.begin(); iter != funcTable.end(); ++iter)
	{

		FuncBlock funcBlock;										// 当前处理的函数块
		priority_queue<int, vector<int>, greater<int>> blockEnters; // 记录所有基本块的入口点，小顶堆保证顺序

		// 将函数的入口点作为第一个基本块的入口
		blockEnters.push(iter->enterPoint);
		// 计算函数的结束点，如果是最后一个函数，则结束点是四元式的总数；否则是下一个函数的开始点
		int endIndex = iter + 1 == funcTable.end() ? this->quaternaries.size() : (iter + 1)->enterPoint;

		// 遍历函数的四元式序列，找出所有基本块的入口点
		for (int index = iter->enterPoint; index < endIndex; index++)
		{
			// 对于跳转指令，需要记录跳转目的地和跳转指令的下一条指令作为新的入口点
			if (quaternaries[index].op[0] == 'j')
			{
				// ① 直接跳转（j）
				if (quaternaries[index].op == "j")
					blockEnters.push(atoi(quaternaries[index].dst.c_str()));
				// 条件跳转
				else
				{
					if (index + 1 < endIndex)
						blockEnters.push(index + 1);						 // 跳转指令的下一条也是一个入口点
					blockEnters.push(atoi(quaternaries[index].dst.c_str())); // 跳转到的指令
				}
			}
			// ② 函数调用和③ 返回指令，下一条指令也是一个新的入口点
			else if (quaternaries[index].op == "return" || quaternaries[index].op == "call")
				blockEnters.push(index + 1);
		}

		// 根据入口点划分基本块
		Block blockTemp;			 // 临时存放正在构建的基本块
		map<int, string> labelEnter; // 入口点与基本块标签的映射
		map<int, int> blockEnter;	 // 入口点与基本块序号的映射

		int firstFlag = true;			   // 标记是否是函数的第一个基本块
		int enter;						   // 当前处理的入口点
		int lastEnter = blockEnters.top(); // 上一个处理的入口点
		blockEnters.pop();

		// 循环处理所有的入口点，生成基本块
		while (!blockEnters.empty())
		{
			// 插入四元式到block中
			enter = blockEnters.top();
			blockEnters.pop();

			// 基本块装填
			if (enter == lastEnter)
				continue;
			else
				for (int i = lastEnter; i != enter; i++)
					blockTemp.codes.push_back(quaternaries[i]);

			// 首块以函数名命名
			if (firstFlag)
			{
				firstFlag = false;
				blockTemp.name = iter->name;
				funcBlock.name = iter->name;							// 函数块名
				funcBlock.serial = blockFuncLabelGenerator.new_label(); // 生成序列号
			}
			// 非首块以自动生成的标签命名
			else
				blockTemp.name = blockLabelGenerator.new_label();

			labelEnter[lastEnter] = blockTemp.name;
			blockEnter[lastEnter] = (int)(funcBlock.blocks.size());

			// 将构建好的基本块添加到函数块中
			funcBlock.blocks.push_back(blockTemp);
			lastEnter = enter;
			blockTemp.codes.clear(); // 清空临时基本块，准备构建下一个
		}

		// 计算每个基本块的下一个基本块（用于控制流分析）
		int blockIndex = 0;
		for (auto &block : funcBlock.blocks)
		{
			auto lastCode = block.codes.rbegin(); // 基本块的最后一条四元式
			// 跳转指令处理
			if (lastCode->op[0] == 'j')
			{
				// 直接跳转
				if (lastCode->op == "j")
				{
					block.next1 = blockEnter[atoi(lastCode->dst.c_str())];
					block.next2 = -1;
				}
				// 条件跳转
				else
				{
					block.next1 = blockIndex + 1;
					block.next2 = blockEnter[atoi(lastCode->dst.c_str())];
					block.next2 = (block.next1 == block.next2) ? -1 : block.next2; // 避免自循环
				}
				lastCode->dst = labelEnter[atoi(lastCode->dst.c_str())]; // 更新跳转目标标签
			}
			// return指令处理
			else if (lastCode->op == "return")
			{
				block.next1 = -1;
				block.next2 = -1;
			}
			// 其他情况，默认顺序执行到下一个基本块
			else
			{
				block.next1 = blockIndex + 1;
				block.next2 = -1;
			}
			blockIndex++;
		}

		// 将处理完毕的函数块添加到最终的函数块列表中
		this->blockFunc.push_back(funcBlock);
	}
	GenerateDotGraphsForAllFuncBlocks();
}
