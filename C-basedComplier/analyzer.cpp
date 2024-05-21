#include "analyzer.h"

Compile_Analyzer::Compile_Analyzer()
{
    this->labelGenerator = LabelGenerator(0, "T");
}

// 获取函数表
vector<Func> Compile_Analyzer::getFunTable(void)
{
    return (this->funcTable);
}

// 绘制树sub【graphics】
void Compile_Analyzer::generateSubtreeShape(ofstream &outputStream, Tree *&node)
{
    if (node->children.size())
        outputStream << node->key << "[shape=polygon, style=filled, fillcolor=\"#f6cec1\", fontname=\"Helvetica-Bold\", label=\"" << symbolStr[node->value] << "\"];" << endl;
    else
        outputStream << node->key << "[shape=ellipse, style=filled, fillcolor=\"#c6e6e8\", fontname=\"Helvetica\", label=\"" << symbolStr[node->value] << "\"];" << endl;
    for (size_t i = 0; i < node->children.size(); ++i)
        generateSubtreeShape(outputStream, node->children[i]);
}

void Compile_Analyzer::generateSubtreeConnections(ofstream &outputStream, Tree *&node)
{
    for (size_t i = 0; i < node->children.size(); ++i)
        outputStream << node->key << "->" << (node->children[i])->key << ";" << endl;
    for (size_t i = 0; i < node->children.size(); ++i)
        generateSubtreeConnections(outputStream, node->children[i]);
}

// 绘制树【graphics】
void Compile_Analyzer::generateSyntaxTree(stack<vector<int>> &parseStack, const char *outputFilePath)
{
    int nodeIdentifier = 0;      // 用来唯一标识某个树节点
    Tree *rootNode;              // 树根节点
    stack<Tree *> VariableStack; // 变量存储栈

    // ---- 根节点 ----
    vector<int> currentProduction = parseStack.top();
    parseStack.pop();
    if (currentProduction.size() == 1) // 空产生式特判
        currentProduction.emplace_back(numVt);
    rootNode = new Tree(nodeIdentifier++, currentProduction[0]);

    for (size_t i = 1; i < currentProduction.size(); ++i)
    {
        Tree *newNode = new Tree(nodeIdentifier++, currentProduction[i]);
        rootNode->children.emplace_back(newNode);
        VariableStack.push(newNode);
    }

    // LR1分析最左规约 --> 最右推导
    while (parseStack.size())
    {
        vector<int> currentProduction = parseStack.top();
        parseStack.pop();

        // 空产生式特判
        if (currentProduction.size() == 1)
            currentProduction.emplace_back(numVt);

        // 寻找第一个非终结符扩展
        Tree *parentNode = VariableStack.top();
        while (parentNode->value >= 0 && parentNode->value <= numVt)
        { // 终结符&空产生式继续
            VariableStack.pop();
            if (!VariableStack.size())
                break;
            parentNode = VariableStack.top();
        }
        if (!VariableStack.size()) // 结束
            break;
        if (parentNode->value != currentProduction[0])
        {
            cerr << "error！" << endl;
            return;
        }
        VariableStack.pop(); // 弹出parentNode

        // 添加节点
        for (size_t i = 1; i < currentProduction.size(); i++)
        {
            Tree *newNode = new Tree(nodeIdentifier++, currentProduction[i]);
            parentNode->children.emplace_back(newNode);
            VariableStack.push(newNode);
        }
    }

    ofstream outputStream(outputFilePath, ios::out);
    outputStream << "digraph SyntaxTree {" << endl;
    outputStream << "rankdir = TB;" << endl;
    generateSubtreeShape(outputStream, rootNode);
    generateSubtreeConnections(outputStream, rootNode);
    outputStream << "}" << endl;
}

// 获取词法分析结果
int Compile_Analyzer::get_words(const char *lexInput)
{
    // 初始化words序列
    tokens.clear();
    int id;
    string temp1, temp2;
    ifstream input(lexInput, ios::in);
    if (!input.is_open())
    {
        cerr << "can not open" << lexInput << "\n";
        Message += "[error] can not open";
        Message += lexInput;
        Message += "\n";
        return RETURN_ERROR;
    }
    while (true)
    {
        input >> id >> temp1 >> temp2;
        if (id == -1)
            break;
        tokens.emplace_back(Token{id, temp1, temp2});
    }
    tokens.emplace_back(Token{0, "0", "0"}); // 结束符号

    return RETURN_FINE;
}

// 从语法分析器获得各项数据信息
int Compile_Analyzer::get_value_from_syn(void)
{
    this->symbolMap = this->syntaxAnalyzer.symbolMap;         // 非终结&终结符转化,1-85为终结符，86-112为非终结符，舍弃旧字符，这里跟原来很不一样
    this->symbolStr = this->syntaxAnalyzer.symbolStr;         // 编号-<str
    this->numVt = this->syntaxAnalyzer.numVt;                 // 终结符个数     85+1=86
    this->numVn = this->syntaxAnalyzer.numVn;                 // 非终结符个数   26
    this->epsilon = this->syntaxAnalyzer.epsilon;             // ε对应的值     86[即非终结符中第一个]
    this->productionNum = this->syntaxAnalyzer.productionNum; // 产生式个数

    return RETURN_FINE;
}

// 从文档数据中获取LR1表相关数据
int Compile_Analyzer::get_LR1_tables(const char *fileProduction, const char *fileTableLR1, const char *fileTableSR)
{
    this->get_value_from_syn();
    this->tableLR1 = this->syntaxAnalyzer.load_LR1Table(fileTableLR1);
    this->shiftReduceTable = this->syntaxAnalyzer.load_s_r_Table(fileTableSR);
    this->production = this->syntaxAnalyzer.load_LR1Production(fileProduction);

    return RETURN_FINE;
}

int Compile_Analyzer::initForAll(void)
{
    // 构建词法分析的字典树
    this->lexicalAnalyzer.initialLexicalAnalysis();

    // 构建LR1表
    return this->syntaxAnalyzer.syntaxAnalyzeLR1Table();
}

int Compile_Analyzer::refresh(void)
{
    this->tokens.clear();
    this->varTable.clear();
    this->funcTable.clear();
    labelGenerator.refresh();
    while (!this->stateStack.empty())
        this->stateStack.pop();
    while (!this->symbolStack.empty())
        this->symbolStack.pop();
    this->lexicalAnalyzer.refresh();
    // syn用于生成LR1Table无需fresh
    this->interCodeGenerator.refresh();
    this->objectCodeGenerator.refresh();

    return RETURN_FINE;
}

int Compile_Analyzer::lexAnalyzer(void)
{
    return this->lexicalAnalyzer.lexialAnalyze();
}

int Compile_Analyzer::synAnalyzer(void)
{
    if (this->get_words() == RETURN_FINE &&
        this->get_LR1_tables() == RETURN_FINE &&
        this->regular_LR1() == RETURN_FINE)
        return RETURN_FINE;
    else
        return RETURN_ERROR;
}

int Compile_Analyzer::semAnalyzer(void)
{
    interCodeGenerator.Output((folderPath + semCode).c_str());
    return RETURN_FINE;
}

int Compile_Analyzer::interAnalyzer(void)
{
    this->interCodeGenerator.DivideIntoFunctionBlocks(this->getFunTable());                    // 划分功能块
    this->objectCodeGenerator.analyseFuncBlocks(this->interCodeGenerator.GetFunctionBlocks()); // 赋值待用活跃信息
    ofstream out;
    out.open((folderPath + intCode).c_str());
    if (!out.is_open())
    {
        return RETURN_ERROR;
    }
    this->objectCodeGenerator.outputInfoBlocksDetails(out);
    return RETURN_FINE;
}

int Compile_Analyzer::objAnalyzer(void)
{
    this->objectCodeGenerator.generateCode();
    this->objectCodeGenerator.outputGeneratedCode((folderPath + objCode).c_str());
    return RETURN_FINE;
}
