#include "syntaxAnalyzer.h"

/* item项目集 */
// 添加边：将一个从项目集from到项目集to的边添加到图中，权重为w。用于构建项目集族之间的转移。
void syntaxAnalyzer::itemSetAddEgde(int from, int to, int w) // 添加边
{
    edges.emplace_back(Edge{head[from], to, w});
    head[from] = edges.size() - 1; // 更新head数组，指向edges中最新添加的边
}

// 判断一个item（项目）是否在给定的项目集中。
inline bool syntaxAnalyzer::itemSetIfInSet(const Item &a, const vector<Item> &b)
{
    for (const auto &item : b)
    {
        if (a.forward == item.forward && a.ppos == item.ppos && a.nump == item.nump)
            return true;
    }
    return false;
}

// 合并两个项目集：将b项目集中的项目合并到a项目集中，如果它们还不存在于a中。
vector<Item> syntaxAnalyzer::itemSetMerge(vector<Item> a, vector<Item> b)
{
    for (const auto &itemB : b)
    {
        if (!itemSetIfInSet(itemB, a))
            a.push_back(itemB);
    }
    return a;
}

// 查找项目集：在给定的项目集族中查找一个项目集，如果找到，返回其编号；否则返回-1。
int syntaxAnalyzer::itemSetSearch(vector<Item> &a, vector<vector<Item>> &b)
{
    for (size_t i = 0; i < b.size(); i++)
    {
        if (a.size() != b[i].size())
            continue; // 如果大小不同，立即继续下一个迭代

        bool allItemsMatch = true;
        for (size_t j = 0; j < a.size(); j++)
        {
            if (!itemSetIfInSet(a[j], b[i]))
            {
                allItemsMatch = false; // 发现不匹配的项目，设置标志为false
                break;
            }
        }

        if (allItemsMatch)
        {
            return i; // 找到完全匹配的项目集，返回其索引
        }
    }
    return -1; // 未找到匹配的项目集，返回-1
}

// 读取并初始化产生式：将文本形式的产生式转换为数字形式，用于内部处理。
vector<int> syntaxAnalyzer::initProduc(const char *_pattern)
{
    vector<int> tmp;           // 用来存储转换后的产生式的数字表示。
    string pattern = _pattern; // 将传入的C风格字符串转换为C++风格的字符串，便于处理。
    // 去除空格
    int i;
    while ((i = pattern.find(' ')) != -1)
    {
        pattern.erase(i, 1);
    }
    // 读取逗号之间的内容
    int startindex = 0;
    int endindex = 0;
    string sub;
    while (pattern.find(',', startindex) != -1)
    {
        endindex = pattern.find(',', startindex);
        // 每个符号可能是终结符或非终结符，在产生式中用字符串表示。
        sub = pattern.substr(startindex, endindex - startindex);
        // 遇到单引号特殊处理（里面都是终结符）
        if (sub[0] == '\'')
        {
            endindex = pattern.find('\'', startindex + 1) + 1;
            sub = pattern.substr(startindex + 1, endindex - startindex - 2);
        }
        // 对每个读取到的符号字符串，使用this->symbolMap[sub]将其转换为预先定义好的数字表示。
        tmp.push_back(this->symbolMap[sub]);
        startindex = endindex + 1;
    }
    // 处理最后一个单词（如果最后一个符号也被单引号包围，则同样去除单引号并转换其为数字表示。）
    sub = pattern.substr(startindex, pattern.length() - startindex);
    if (sub[0] == '\'')
    {
        endindex = pattern.find('\'', startindex + 1) + 1;
        sub = pattern.substr(startindex + 1, endindex - startindex - 2);
    }
    tmp.push_back(this->symbolMap[sub]);
    return tmp;
}

// 准备分析：初始化语法分析器内部使用的各种数据结构和变量。
void syntaxAnalyzer::initAnalyzer()
{
    // 初始化个数
    this->numVt = sizeof(VTNAMES) / sizeof(VTNAMES[0]);
    this->numVn = sizeof(VNNAMES) / sizeof(VNNAMES[0]);
    this->productionNum = sizeof(PATTERNS) / sizeof(PATTERNS[0]);

    // 初始化转换数组
    // Map和Vstr是用于存储终结符和非终结符的映射。Map是从字符串到整数的映射，而Str是从整数到字符串的映射。
    symbolMap.clear();
    symbolStr.clear();

    // 语法分析过程中用来表示输入结束或栈底。
    symbolMap["##"] = 0; // 栈最后的#号
    symbolStr.push_back("##");

    int index = 1;
    // 终结符
    for (int i = 0; i < numVt; ++i)
    {
        symbolStr.push_back(string(VTNAMES[i]));
        symbolMap[string(VTNAMES[i])] = index++;
    }
    // 非终结符
    for (int i = 0; i < numVn; ++i)
    {
        symbolStr.push_back(string(VNNAMES[i]));
        symbolMap[string(VNNAMES[i])] = index++;
    }

    // 【调整参数episilon变化为终结符】
    // 构建LR(1)分析表时，需要将空串作为一个特殊的终结符处理。
    numVt++;               // 86[##-0 + VT-1~85]
    this->epsilon = numVt; // epsilon-86 即 emptypro

    // production->产生式的数字表示；getProduce->终结符对应产生式的编号
    production.clear();
    getProduce.clear();
    getProduce.resize(numVt + numVn, "");

    for (int i = 0; i < productionNum - 1; i++)
    {
        // 遍历PATTERNS数组，将文本形式的产生式转换为数字表示，并添加到production中。
        vector<int> tmp = initProduc(PATTERNS[i]); // 先不检查错误回头再看
        getProduce[tmp[0]] += ('\0' + i);          // 每个非终结符对应的产生式编号
        production.push_back(tmp);
    }
}

// 计算First集合的辅助函数：对给定的字符串进行unique_add操作，确保结果集中没有重复元素。
string addB2A(string str_a, string str_b)
{
    string ans = str_a;
    unordered_set<char> chars(str_a.begin(), str_a.end());
    for (char ch : str_b)
    {
        if (chars.find(ch) == chars.end())
        {
            ans += ch;
            chars.insert(ch); // 更新哈希集合，以防b中有重复字符
        }
    }
    return ans;
}

string addB2A(string str_a, char ch_b)
{
    string ans = str_a;
    if (str_a.find(ch_b) == size_t(-1))
        ans += ch_b;
    return ans;
}

// 深度优先搜索计算First集合：对给定的非终结符和产生式编号进行DFS，计算其First集。
void syntaxAnalyzer::dfs(int nv, int nump, vector<bool> &getted)
{
    getted[nump] = true; // 标记此产生式已处理

    int symbol = production[nump][1]; // 产生式推出来的首符
    // 先处理终结符
    if (symbol <= numVt)
    {
        first[nv] = addB2A(first[nv], char('\0' + symbol));
        return;
    }

    // 再处理非终结符
    for (size_t j = 1; j < production[nump].size(); ++j)
    {
        symbol = production[nump][j];

        // 所有symbol可推导出的符号对应的产生式
        for (size_t i = 0; i < getProduce[symbol].size(); ++i)
        {
            int prodIndex = getProduce[symbol][i] - '\0';
            if (production[prodIndex][0] == production[prodIndex][1])
                continue; // 忽略左递归产生式

            dfs(symbol, prodIndex, getted);
        }
        /*
        如果在某个产生式中遇到了能够导出ε的非终结符（即其First集包含ε），则需要特殊处理：
            如果除了ε之外还有其他符号，那么这些符号也需要被加入到nv的First集中。
            如果所有符号都能导出ε，那么ε也需要被加入到nv的First集中。
        */
        int pos = first[symbol].find('\0' + epsilon);
        // 没有找到空字符ε，直接将symbol的First集，除ε以外，全部加入到nv的First集中
        if (pos == std::string::npos)
        {
            first[nv] = addB2A(first[nv], first[symbol]);
            break; // 一旦遇到不能导出ε的符号，停止后续处理
        }
        // 如果除了ε之外还有其他符号，也需要加入到nv的First集中
        else
        {
            first[nv] = addB2A(first[nv], first[symbol]);
            // 只有当处理到最后一个符号，且该符号可以导出ε时，才将ε加入到nv的First集中
            if (j == production[nump].size() - 1)
            {
                first[nv] = addB2A(first[nv], '\0' + epsilon);
            }
        }
    }
}

// 计算所有符号的First集合。
void syntaxAnalyzer::getFirstSet()
{
    // 初始化first数组
    vector<bool> getted; // 用来标记每个产生式是否已经计算了First集合。
    getted.resize(productionNum, false);
    first.clear();
    first.resize(numVt + numVn, "");

    // 终结符的first就是他自己，null没有first
    for (int i = 1; i <= numVt; i++)
    {
        first[i] += char('\0' + i);
    }

    // 直接处理空产生式，避免在后续dfs过程中重复检查
    for (size_t i = 0; i < production.size(); i++)
    {
        if (production[i][1] == epsilon && production[i].size() == 2)
        {
            production[i].pop_back(); // 移除空产生式右侧的ε标记
        }
    }

    // 非终结符
    for (size_t i = 0; i < production.size(); i++)
    {
        // 已经生成或左递归的产生式不用，不影响求First集
        if (!getted[i] && production[i][0] != production[i][1])
        {
            dfs(production[i][0], i, getted);
        }
    }
}

// 计算给定项目的闭包：对给定的项目进行闭包运算，得到包含所有可达项目的集合
vector<Item> syntaxAnalyzer::getItemClosure(Item item)
{
    vector<Item> closureSet;
    closureSet.push_back(item);
    queue<Item> processingQueue;
    processingQueue.push(item);

    while (!processingQueue.empty())
    {
        Item currentItem = processingQueue.front();
        processingQueue.pop();
        // 归约项舍去
        if ((size_t)currentItem.ppos == production[currentItem.nump].size())
            continue;

        // 是'.'之后的符号
        int nextSymbol = production[currentItem.nump][currentItem.ppos];

        if (nextSymbol <= numVt)
            continue; // 如果是终结符，跳过

        // 若是非终结符，对应产生式的编号
        for (size_t i = 0; i < getProduce[nextSymbol].size(); ++i)
        {
            Item newItem;
            newItem.ppos = 1;                                // .一定在最前面
            newItem.nump = getProduce[nextSymbol][i] - '\0'; // 该产生式的编号
            // 处理[S->a.B, c]情况
            if (currentItem.ppos + 1 == production[currentItem.nump].size())
                newItem.forward += currentItem.forward;
            // 处理[S->a.BC...d, c]情况
            else
            {
                for (size_t j = 1; j < production[currentItem.nump].size(); ++j)
                {
                    int nxt = production[currentItem.nump][currentItem.ppos + j];
                    int pos = first[nxt].find('\0' + epsilon);
                    // 没有空字符ε
                    if (pos == -1)
                    {
                        newItem.forward += first[nxt];
                        break;
                    }
                    // first集包含ε，需要排除ε
                    newItem.forward += first[nxt].substr(0, pos);
                    newItem.forward += first[nxt].substr(pos + 1);
                    // 后面的first全都包含ε
                    if (currentItem.ppos + j == production[currentItem.nump].size() - 1)
                    {
                        newItem.forward += currentItem.forward;
                        break;
                    }
                }
            }
            if (!itemSetIfInSet(newItem, closureSet))
            {
                processingQueue.push(newItem);
                closureSet.push_back(newItem);
            }
        }
    }
    return closureSet;
}

// 从初始项目集开始，逐步扩展，构建整个项目集簇
void syntaxAnalyzer::getItemSet()
{
    head.resize(MAXN, -1);
    vector<Item> initial;

    Item startItem{0, 1, string(1, '\0')}; // 精简初始化方式
    initial = getItemClosure(startItem);   // 获得初始项目集闭包
    queue<vector<Item>> q;                 // 项目集组队列
    q.push(initial);
    itemSet.push_back(initial); // 项目集族   //S -> .BB, #

    while (!q.empty())
    {
        vector<Item> curItemSet = q.front();
        q.pop();

        for (int i = 1; i < numVt + numVn; ++i)
        {
            if (i == epsilon)
                continue;

            vector<Item> tmpItemSet; // 该项目集中的所有项目寻找新的项目集
            for (const auto &item : curItemSet)
            {
                if (static_cast<size_t>(item.ppos) < production[item.nump].size())
                {

                    int tt = production[item.nump][item.ppos];
                    if (tt == i)
                    {
                        Item tempt{item.nump, item.ppos + 1, item.forward};
                        tmpItemSet = itemSetMerge(tmpItemSet, getItemClosure(tempt));
                    }
                }
            }
            // 该符号无法读入
            if (tmpItemSet.size() == 0)
                continue;
            int currentIndex = itemSetSearch(curItemSet, itemSet); // 查找当前项目集在项目集族中的索引
            int nextIndex = itemSetSearch(tmpItemSet, itemSet);    // 查找临时项目集在项目集族中的索引，如果不存在则为-1
            // 添加项目集族的边
            if (nextIndex == -1)
            {                                                        // 如果临时项目集在项目集族中不存在，则需要添加新的项目集
                itemSet.push_back(tmpItemSet);                       // 将临时项目集添加到项目集族中
                q.push(tmpItemSet);                                  // 将临时项目集加入队列，以便后续处理
                itemSetAddEgde(currentIndex, itemSet.size() - 1, i); // 添加从当前项目集到新添加项目集的边
            }
            // 如果临时项目集已存在于项目集族中，只需添加边，不需要重复添加项目集
            else
                itemSetAddEgde(currentIndex, nextIndex, i);
        }
    }
}

void syntaxAnalyzer::printItemSetDetails() const
{
    auto filename = (srcPath + "itemSet.txt").c_str();
    ofstream out(filename);
    if (!out.is_open())
    {
        cerr << "无法打开文件 " << filename << "\n";
        Message += "【错误】 无法打开文件！";
        Message += filename;
        Message += "\n";
        return;
    }

    for (size_t i = 0; i < itemSet.size(); i++)
    {
        out << "项目集I" << i << ":" << std::endl;
        for (const auto &item : itemSet[i])
        {
            // 输出产生式左侧的非终结符
            out << "{ " << symbolStr[production[item.nump][0]] << " -> ";
            // 输出产生式右侧，包括'.'
            for (size_t k = 1; k < production[item.nump].size(); ++k)
            {
                if (k == (size_t)item.ppos)
                    out << ". ";
                out << symbolStr[production[item.nump][k]] << " ";
            }
            if ((size_t)item.ppos == production[item.nump].size())
                out << ". ";
            // 输出向前看符号
            out << "}, ";
            for (auto ch : item.forward)
            {
                int index = ch - '\0'; // ch以编码形式存储的字符，'\0'是基准
                if (index >= 0 && (size_t)index < symbolStr.size())
                    out << symbolStr[index] << ", ";
                else
                    out << ch;
            }
            out << endl;
        }
        out << endl;
    }
}

// 根据构建的项目集族和转移关系，生成LR(1)分析表【table[i][j] = w:状态i --> j，读入符号W】
bool syntaxAnalyzer::genLR1Table()
{
    // 初始化表格大小和默认值
    auto initializeTable = [](auto &table, int size, int defaultValue)
    {
        table.resize(size);
        for (auto &row : table)
        {
            row.resize(size, defaultValue);
        }
    };

    // 初始化LR1和SR表
    initializeTable(tableLR1, MAXN, TBSTATE_NONE);
    initializeTable(shiftReduceTable, MAXN, TBSTATE_NONE);

    // 遍历项目集的边，设置移进项
    for (size_t i = 0; i < itemSet.size(); ++i)
    {
        for (auto j = head[i]; j != -1; j = edges[j].from)
        {
            auto &edge = edges[j];
            if (tableLR1[i][edge.weight] != TBSTATE_NONE)
                return false;                                 // 多重入口，报错
            tableLR1[i][edge.weight] = edge.toItemSet;        // 移进
            shiftReduceTable[i][edge.weight] = TBSTATE_SHIFT; // 移进项
        }
    }

    // 遍历所有项目，设置归约和接受项
    for (size_t i = 0; i < itemSet.size(); ++i)
    {
        for (auto &item : itemSet[i])
        {
            if ((size_t)item.ppos == production[item.nump].size())
            { // 归约项
                for (auto ch : item.forward)
                {
                    auto index = ch - '\0';
                    if (tableLR1[i][index] != TBSTATE_NONE)
                        return false; // 多重入口
                    tableLR1[i][index] = (ch == '\0' && item.nump == 0) ? TBSTATE_ACC : item.nump;
                    shiftReduceTable[i][index] = (ch == '\0' && item.nump == 0) ? TBSTATE_NONE : TBSTATE_REGULAR;
                }
            }
        }
    }
    return true;
}

void syntaxAnalyzer::printLR1Table(const char *filename)
{
    ofstream out(filename);
    if (!out.is_open())
    {
        cerr << "can not open file " << filename << "\n";
        Message += "[error] can not open file ";
        Message += filename;
        Message += "\n";
        return;
    }
    out << itemSet.size() << " " << numVt + numVn << endl;

    for (int j = 0; j < numVt + numVn; j++)
    {
        out << "    " << symbolStr[j];
    }
    out << endl;
    for (size_t i = 0; i < itemSet.size(); i++)
    {
        for (size_t j = 0; j < numVt + numVn; j++)
        {
            if ((size_t)j == numVt)
                continue;
            if (tableLR1[i][j] == TBSTATE_ACC)
                out << "acc" << ' '; // 接受
            else if (tableLR1[i][j] == TBSTATE_NONE)
                out << "*" << ' '; // 空
            else if (shiftReduceTable[i][j] == TBSTATE_SHIFT)
                out << "s" << tableLR1[i][j] << ' '; // 移进
            else if (shiftReduceTable[i][j] == TBSTATE_REGULAR)
                out << "r" << tableLR1[i][j] << ' '; // 归约
        }
        out << endl;
    }
}

void syntaxAnalyzer::genActionGOTOTable(const char *filename)
{
    ofstream out(filename);
    if (!out.is_open())
    {
        cerr << "can not open file " << filename << "\n";
        Message += "[error] can not open file ";
        Message += filename;
        Message += "\n";
        return;
    }
    out << itemSet.size() << " " << numVt + numVn << endl;

    for (int j = 0; j < numVt + numVn; j++)
    {
        out << "    " << symbolStr[j];
    }
    out << endl;
    for (int i = 0; i < itemSet.size(); i++)
    {
        for (int j = 0; j < numVt + numVn; j++)
        {
            if (j >= numVt)
            {
                if (tableLR1[i][j] != TBSTATE_NONE)
                {
                    out << tableLR1[i][j] << ' ';
                }
                else
                    out << "*" << ' ';
            }
            else if (tableLR1[i][j] == TBSTATE_ACC)
                out << "acc" << ' '; // 接受
            else if (tableLR1[i][j] == TBSTATE_NONE)
                out << "*" << ' '; // 空
            else if (shiftReduceTable[i][j] == TBSTATE_SHIFT)
                out << "s" << tableLR1[i][j] << ' '; // 移进
            else if (shiftReduceTable[i][j] == TBSTATE_REGULAR)
                out << "r" << tableLR1[i][j] << ' '; // 归约
        }
        out << endl;
    }
}

void syntaxAnalyzer::genActionGOTOTableCSV(const char *filename)
{
    ofstream out(filename);
    if (!out.is_open())
    {
        cerr << "Cannot open file " << filename << "\n";
        return;
    }

    // 打印表头
    out << "State";
    for (int j = 0; j < numVt + numVn; ++j)
    {
        out << ","
            << "\"" << symbolStr[j] << "\""; // 字段用逗号分隔，字段名用引号包围
    }
    out << "\n";

    // 打印表内容
    for (int i = 0; i < itemSet.size(); ++i)
    {
        out << i; // 当前状态编号
        for (int j = 0; j < numVt; ++j)
        { // Action表部分
            out << ",";
            if (tableLR1[i][j] == TBSTATE_ACC)
                out << "\"acc\""; // 接受
            else if (tableLR1[i][j] == TBSTATE_NONE)
                out << "\"*\""; // 空
            else if (shiftReduceTable[i][j] == TBSTATE_SHIFT)
                out << "\"s" << tableLR1[i][j] << "\""; // 移进
            else if (shiftReduceTable[i][j] == TBSTATE_REGULAR)
                out << "\"r" << tableLR1[i][j] << "\""; // 归约
        }
        for (int j = numVt; j < numVt + numVn; ++j)
        { // GOTO表部分
            out << ",";
            if (tableLR1[i][j] != TBSTATE_NONE)
                out << tableLR1[i][j]; // 直接输出状态编号
            else
                out << "\"*\""; // 空
        }
        out << "\n";
    }

    out.close();
}

// 将LR1表的信息存储到文件中
void syntaxAnalyzer::genLR1Table_2(const char *filename)
{
    int row = itemSet.size(), col = numVt + numVn;
    ofstream out(filename);
    out << row << " " << col << endl;

    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            out << tableLR1[i][j] << " ";
        }
        out << endl;
    }
}

// 将tableSR表的信息存储到文件中
void syntaxAnalyzer::genSRTable(const char *filename)
{
    int row = itemSet.size(), col = numVt + numVn;
    ofstream out(filename);
    out << row << " " << col << endl;
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            out << shiftReduceTable[i][j] << " ";
        }
        out << endl;
    }
}

// 将产生式存储到文件中
void syntaxAnalyzer::genLR1Production(const char *filename)
{
    ofstream out(filename);
    out << production.size() << endl;
    for (auto it = production.begin(); it != production.end(); it++)
    {
        out << it->size() << ' ';
        for (auto itt = it->begin(); itt != it->end(); itt++)
        {
            out << *itt << " ";
        }
        out << endl;
    }
}

// 从文件中读取LR1表的信息
vector<vector<int>> syntaxAnalyzer::load_LR1Table(const char *filename)
{
    tableLR1.resize(MAXN);
    for (int i = 0; i < tableLR1.size(); i++)
    {
        tableLR1[i].resize(MAXN, TBSTATE_NONE);
    }
    int row, col;
    ifstream in(filename, ios::in);
    in >> row >> col;
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            in >> tableLR1[i][j];
        }
    }

    return this->tableLR1;
}
// 从文件中读取tableSR表的信息
vector<vector<int>> syntaxAnalyzer::load_s_r_Table(const char *filename)
{
    shiftReduceTable.resize(MAXN);
    for (int i = 0; i < shiftReduceTable.size(); i++)
    {
        shiftReduceTable[i].resize(MAXN, TBSTATE_NONE);
    }
    int row, col;
    ifstream in(filename, ios::in);
    in >> row >> col;
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            in >> shiftReduceTable[i][j];
        }
    }

    return this->shiftReduceTable;
}

// 从文件中读取产生式的信息
vector<vector<int>> syntaxAnalyzer::load_LR1Production(const char *filename)
{
    ifstream in(filename);
    production.clear();
    int row, num, k;
    in >> row;
    for (int i = 0; i < row; i++)
    {
        vector<int> tmp;
        in >> num;
        for (int j = 0; j < num; j++)
        {
            in >> k;
            tmp.push_back(k);
        }
        production.push_back(tmp);
    }

    return this->production;
}

int syntaxAnalyzer::syntaxAnalyzeLR1Table(const char *LR1show, const char *LR1, const char *LR1SR, const char *LR1Produncton)
{
    this->initAnalyzer();
    this->getFirstSet();
    this->getItemSet();
    this->printItemSetDetails();

    if (!this->genLR1Table())
    {
        cout << "该文法不是LR（1）文法！" << endl;
        Message += "【错误】 该文法不是LR（1）文法！\n";
        return RETURN_ERROR;
    }
    this->printLR1Table(LR1show); // 输出LR1分析表
    this->genActionGOTOTable((srcPath + "ActionGO.txt").c_str());
    this->genActionGOTOTableCSV((srcPath + "ActionGO.csv").c_str());
    this->genLR1Table_2(LR1);              // 输出Table表
    this->genSRTable(LR1SR);               // 输出Table_s_r表
    this->genLR1Production(LR1Produncton); // 输出产生式

    return RETURN_FINE;
}
