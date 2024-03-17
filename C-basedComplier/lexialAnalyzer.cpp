//@func   :  词法分析器函数

#include "lexialAnalyzer.h"

map<string, int> dic; // 字典，根据某个字符查询到对应的数组下标

//@func : 根据前后排序
bool cmp(lexWord l, lexWord r)
{
    return l.start < r.start;
}

//@func : 构造函数
PrefixTree::PrefixTree()
{
    isEndOfWord = false;
    memset(children, NULL, sizeof(children));
}

//@func : 字符串插入
void PrefixTree::insert(const string &word, const int &t)
{
    PrefixTree *node = this;
    for (char ch : word)
    {
        if (node->children[ch - '\0'] == NULL)
            node->children[ch - '\0'] = new PrefixTree();
        node = node->children[ch - '\0'];
    }
    node->isEndOfWord = true;
    node->wordType = t;
}

vector<lexWord> PrefixTree::searchWords(const string &word)
{
    int len = word.length();
    vector<lexWord> matches;
    int start = 0;

    while (start < len)
    {
        PrefixTree *node = this;
        int i = start;
        PrefixTree *lastMatchNode = nullptr; // 保存最后一个匹配的节点
        int matchEnd = start;                // 保存匹配结束的位置

        for (; i < len; ++i)
        {
            char ch = word[i];
            node = node->children[ch - '\0'];

            if (!node)
                break; // 没有下一个匹配的字符，结束内层循环

            if (node->isEndOfWord)
            {
                lastMatchNode = node;
                matchEnd = i + 1; // 匹配到的结束位置为当前字符位置 + 1
            }
        }

        if (lastMatchNode)
        {
            // 特殊处理：匹配到的是+或-，检查是否为浮点数的一部分
            if ((lastMatchNode->wordType == dic["+"] || lastMatchNode->wordType == dic["-"]) && i >= 2 && (word[i - 2] == 'e' || word[i - 2] == 'E'))
            {
                // 检查e之前是否全为数字或小数点
                if (all_of(word.begin() + start, word.begin() + i - 2, [](char ch)
                           { return isdigit(ch) || ch == '.'; }))
                {
                    matches.emplace_back(word.substr(start, matchEnd - start + 1), start, matchEnd, lastMatchNode->wordType);
                    start = matchEnd; // 更新start为匹配结束的位置
                    continue;
                }
            }

            // 添加匹配到的词
            matches.emplace_back(word.substr(start, matchEnd - start + 1), start, matchEnd, lastMatchNode->wordType);
            start = matchEnd; // 更新start为匹配结束的位置
        }
        else
            start++; // 没有匹配到，start向前移动一位
    }

    return matches;
}

//-----------------------------------------------------------------------

//@func :
int lexialAnalyzer::initialLexicalAnalysis() // 初始化
{
    // 初始化符号字典
    for (int i = 0; i < VTSIZE; ++i)
        dic.emplace(VTNAMES[i], i);

    // 初始化字典树
    for (int i = dic["{"]; i <= dic["while"]; ++i)
        trie.insert(VTNAMES[i], i);
    return 1;
}

//@func :
int lexialAnalyzer::refresh(void)
{
    if (charTypes != NULL)
    {
        delete[] charTypes;
        charTypes = NULL;
    }
    this->sourceCode.clear();
    this->sourceCodeLength = 0;
    this->fragments.clear();

    return RETURN_FINE;
}

//@func : 设置文本对应的片段属性
void lexialAnalyzer::spiltType(string str, int st, int ed, int type)
{
    fragments.emplace_back(str, st, ed, type); // 再看看
    fill(charTypes + st, charTypes + ed, type);
}

//@func : 处理注释，字符，字符串，分隔符
void lexialAnalyzer::preProcess()
{
    bool inSingleLineComment = false;
    bool inMultiLineComment = false;
    bool inString = false;
    bool inChar = false;
    bool inMacro = false;
    int start = 0;

    for (int i = 0; i <= sourceCodeLength; ++i)
    {
        char ch = sourceCode[i];

        // 单行注释 / 宏定义
        if (inSingleLineComment)
        {
            if (ch == '\0' || ch == '\n')
            {
                if (inMacro)
                {
                    // string str = sourceCode[start];
                    // std::string str(sourceCode + start, sourceCode + i);
                    spiltType(sourceCode.substr(start, i - start + 1), start, i, dic["Macro"]);
                    inSingleLineComment = inMacro = false;
                }
                else
                {
                    spiltType(sourceCode.substr(start, i - start + 1), start, i, dic["Comment"]);
                    inSingleLineComment = 0;
                }
                spiltType(sourceCode.substr(i, 1), i, i + 1, dic["EndLine"]);
            }
        }
        // 多行注释
        else if (inMultiLineComment)
        {
            if (ch == '/' && i != 0 && sourceCode[i - 1] == '*')
            {
                spiltType(sourceCode.substr(start, i - start + 1), start, i + 1, dic["Comment"]);
                inMultiLineComment = false;
            }
        }
        // 字符串处理
        else if (inString)
        {
            if (ch == '\"' && sourceCode[i - 1] != '\\')
            {
                spiltType(sourceCode.substr(start, i - start + 1), start, i + 1, dic["String"]);
                inString = false;
            }
        }
        // 字符处理
        else if (inChar)
        {
            if (ch == '\'' && sourceCode[i - 1] != '\\')
            {
                spiltType(sourceCode.substr(start, i - start + 1), start, i + 1, dic["Character"]);
                inChar = false;
            }
        }
        // 宏定义
        else if (inMacro)
        {
            inMultiLineComment = 1;
        }
        // 单行注释
        else if (ch == '/' && i != 0 && sourceCode[i - 1] == '/')
        {
            start = i - 1;
            inSingleLineComment = 1;
        }
        // 多行注释
        else if (ch == '*' && i != 0 && sourceCode[i - 1] == '/')
        {
            start = i - 1;
            inMultiLineComment = 1;
        }
        else
            switch (ch)
            {
            case '/':
                if (i != 0 && sourceCode[i - 1] == '/')
                {
                    start = i - 1;
                    inSingleLineComment = true;
                }
                else if (i != 0 && sourceCode[i - 1] == '*')
                {
                    inMultiLineComment = true; // This should be handled above
                }
                break;
            case '*':
                if (i != 0 && sourceCode[i - 1] == '/')
                {
                    start = i - 1;
                    inMultiLineComment = true;
                }
                break;
            case '\"':
                start = i;
                inString = true;
                break;
            case '\'':
                start = i;
                inChar = true;
                break;
            case '#':
                start = i;
                inMacro = true;
                break;
            case ' ':
            case '\t':
                spiltType(sourceCode.substr(i, 1), i, i + 1, dic["WhiteSpace"]);
                break;
            case '\n':
                spiltType(sourceCode.substr(i, 1), i, i + 1, dic["EndLine"]);
                break;
            case '\0':
                return;
            }
    }
}

bool lexialAnalyzer::isBoolean(const string &str)
{
    if (str == "true" || str == "false")
        return true;
    return false;
}

//@func : 判别无符号整数,认为有前导0的整数合法
bool lexialAnalyzer::isInt(const string &str)
{
    int state = 0;
    for (size_t i = 0; i < str.length(); ++i)
    {
        char ch = str[i];
        if (state == 0)
        {
            if (ch >= '0' && ch <= '9')
                state = 1;
            else
                break;
        }
        else if (state == 1)
        {
            if (ch >= '0' && ch <= '9')
                state = 1;
            else if (ch == ' ')
                state = 3;
            else
                state = 2;
        }
    }
    return (state == 3);
}

//@func : 判别浮点数
bool lexialAnalyzer::isFloat(const string &str)
{
    char state = '0'; // 初始状态
    int i = 0, n = (int)(str.length());
    char ch = str[i];
    while (state != '8' && state != '9' && i < n)
    {
        ch = str[i++];
        switch (state)
        {
        case '0':
            if ((ch >= '1') && (ch <= '9'))
                state = '1'; // 1-9，转状态 1
            else if (ch == '0')
                state = '2'; // 0，转状态 2
            else
                state = '8'; // 其他，转状态 8
            break;
        case '1':
            if ((ch >= '0') && (ch <= '9'))
                state = '1'; // 0-9，转状态 1
            else if (ch == 'e' || ch == 'E')
                state = '5'; // e ，转状态 5
            else if (ch == '.')
                state = '3'; //. ，转状态 3
            else if (ch == ' ')
                state = '9'; // 结束，转 9
            else
                state = '8'; // 其他，转状态 8
            break;
        case '2':
            if (ch == '.')
                state = '3'; //. ，转状态 3
            else
                state = '8'; // 其他，转状态 8
            break;
        case '3':
            if ((ch >= '0') && (ch <= '9'))
                state = '4'; // 0-9，转状态 4
            else
                state = '8'; // 其他，转状态 8
            break;
        case '4':
            if ((ch >= '0') && (ch <= '9'))
                state = '4'; // 0-9，转状态 4
            else if (ch == 'e' || ch == 'E')
                state = '5'; // e ，转状态 5
            else if (ch == ' ')
                state = '9'; // 空白，转 9
            else
                state = '8'; // 其他，转状态 8
            break;
        case '5':
            if ((ch >= '0') && (ch <= '9'))
                state = '7'; // 0-9，转状态 7
            else if (ch == '+' || ch == '-')
                state = '6'; //+/-，转状态 6
            else
                state = '8'; // 其他，转状态 8
            break;
        case '6':
            if ((ch >= '0') && (ch <= '9'))
                state = '7'; // 0-9，转状态 7
            else
                state = '8'; // 其他，转状态 8
            break;
        case '7':
            if ((ch >= '0') && (ch <= '9'))
                state = '7'; // 0-9，转状态 7
            else if (ch == ' ')
                state = '9'; // 空白，转 9
            else
                state = '8'; // 其他，转状态 8
            break;
        }
    }
    return (state == '9');
}

//@func : 判别标识符
bool lexialAnalyzer::isIdentifier(const string &str)
{
    int state = 0;
    for (size_t i = 0; i < str.length(); ++i)
    {
        char ch = str[i];
        if (state == 0)
        {
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_')
                state = 1;
            else
                break;
        }
        else if (state == 1)
        {
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_' || (ch >= '0' && ch <= '9'))
                state = 1;
            else if (ch == ' ')
                state = 2;
            else
                break;
        }
    }
    return (state == 2);
}

//@func : 字符分析
void lexialAnalyzer::lexialAnalyzeToken(const string &code)
{
    //	this->refresh();

    if (charTypes != NULL)
    { // 文件中的每一个字符对应的类型
        delete[] charTypes;
        charTypes = NULL;
    }
    this->sourceCode.clear();
    this->fragments.clear();
    this->sourceCodeLength = 0;

    sourceCode = code;
    sourceCodeLength = (int)(sourceCode.length());
    charTypes = new int[sourceCodeLength + 1];
    // 初始化
    for (int i = 0; i <= sourceCodeLength; i++)
    {
        charTypes[i] = dic["NULL"];
    }
    preProcess();

    string temp = sourceCode;
    for (int i = 0; i < sourceCodeLength; i++)
    {
        if (charTypes[i])
            temp[i] = ' '; // 用空格替代已处理过的字符
    }

    vector<lexWord> res = trie.searchWords(temp); // 假设存在的字符串匹配函数

    for (const auto &match : res)
        if (match.lexType >= dic["{"] && match.lexType <= dic[";"])
            spiltType(sourceCode.substr(match.start, match.end - match.start + 1), match.start, match.end, match.lexType); // 标记为已处理

    for (const auto &match : res)
    {
        if (match.lexType >= dic["auto"] && match.lexType <= dic["while"])
        {
            // 检查关键字前后是否有相邻的字符
            bool isIsolated = (match.start == 0 || charTypes[match.start - 1]) &&
                              (match.end == sourceCodeLength || charTypes[match.end]);
            if (isIsolated)
            {
                spiltType(sourceCode.substr(match.start, match.end - match.start + 1), match.start, match.end, match.lexType); // 标记为已处理
            }
        }
    }

    int start = -1;
    for (int i = 0; i <= sourceCodeLength; i++)
    {
        char ch = sourceCode[i];
        if (ch != '\0' && !charTypes[i])
        {
            if (start == -1)
                start = i; // 标记词的开始
        }
        else if (start >= 0 && (ch == '\0' || charTypes[i]))
        {
            string str = sourceCode.substr(start, i - start) + ' ';
            if (isBoolean(str))
                spiltType(sourceCode.substr(start, i - start + 1), start, i, dic["Boolean"]);
            else if (isInt(str))
                spiltType(sourceCode.substr(start, i - start + 1), start, i, dic["Integer"]);
            else if (isFloat(str))
                spiltType(sourceCode.substr(start, i - start + 1), start, i, dic["Floating"]);
            else if (isIdentifier(str))
                spiltType(sourceCode.substr(start, i - start + 1), start, i, dic["Identifier"]);
            else
                spiltType(sourceCode.substr(start, i - start + 1), start, i, dic["Word"]);
            start = -1;
        }

        if (ch == '\0')
            break;
    }
}

//@func : 显示词法分析结果
int lexialAnalyzer::stLexAns(ofstream &outFile, ofstream &outFile2, string buffer, vector<lexWord> &res)
{
    for (size_t i = 0; i < res.size(); i++)
    {
        // 空格 / 换行不考虑
        if (!(res[i].lexType == dic["WhiteSpace"] || res[i].lexType == dic["EndLine"]))
        {
            outFile << buffer.substr(res[i].start, res[i].end - res[i].start) << " --> " << VTNAMES[res[i].lexType] << ' ' << buffer.substr(res[i].start, res[i].end - res[i].start) << endl;
            // 宏定义 / 注释不考虑
            if (!(res[i].lexType == dic["Macro"] || res[i].lexType == dic["Comment"]))
                outFile2 << res[i].lexType << ' ' << VTNAMES[res[i].lexType] << ' ' << buffer.substr(res[i].start, res[i].end - res[i].start) << endl;
        }
    }
    return 0;
}

//@func : 词法分析
int lexialAnalyzer::lexialAnalyze(const char *inPath, const char *outPath, const char *outPath2)
{
    // 打开文件
    ifstream inFile(inPath, ios::in);
    ofstream outFile(outPath, ios::out);
    ofstream outFile2(outPath2, ios::out);
    ofstream lexAnalyzeShow_Token((folderPath + "lexOutputToken.txt").c_str(), ios::out);
    ofstream lexAnalyzeShow_Type((folderPath + "lexOutputType.txt").c_str(), ios::out);
    if (!inFile.is_open() || !outFile.is_open() || !outFile2.is_open())
    {
        if (!inFile.is_open())
        {
            cerr << "【警告】无法打开" << inPath << "文件！\n";
            Message += "【警告】 无法打开 ";
            Message += inPath;
            Message += "文件！\n";
        }
        else if (!outFile.is_open())
        {
            cerr << "【警告】无法打开" << outPath << "文件！\n";
            Message += "【警告】 无法打开 ";
            Message += outPath;
            Message += "文件！\n";
        }
        else if (!outFile2.is_open())
        {
            cerr << "c【警告】无法打开" << outPath2 << "文件！\n";
            Message += "【警告】 无法打开 ";
            Message += outPath2;
            Message += "文件！\n";
        }
        return 0;
    }

    // 处理文件
    int ifCommentsError = 0;   // 多行注释错误处理记录
    int wordErrorTypeSign = 0; // word类型出现报错
    string bufLex = "";        // 读入缓冲区
    string buf = "";           // 读入缓冲区
    vector<lexWord> res;

    while (getline(inFile, buf))
    {
        buf += "\n"; // 补足getline未读入的换行符

        for (size_t i = 0; i < buf.size(); ++i)
        {
            if (buf[i] == '/')
            {
                if ((size_t)i < buf.size() - 1 && buf[i + 1] == '*')
                { // 针对/+*
                    ifCommentsError -= 1;
                    i++;
                }
            }
            else if (buf[i] == '*')
            {
                if ((size_t)i < buf.size() - 1 && buf[i + 1] == '/')
                { // 针对*+/
                    if (ifCommentsError < 0)
                        ifCommentsError += 1;
                    i++;
                }
            }
        }
        bufLex += buf;

        if (!ifCommentsError)
        {
            this->lexialAnalyzeToken(bufLex);
            res = fragments;
            map<int, std::string> typeToString;
            for (const auto &pair : dic)
            {
                typeToString[pair.second] = pair.first;
            }

            for (const auto &word : res)
            {
                if (typeToString[word.lexType] == "WhiteSpace" || typeToString[word.lexType] == "EndLine")
                    continue;

                string lexStr = word.lex.substr(0, word.lex.size() - 1);
                istringstream stream(lexStr);
                string line;

                while (std::getline(stream, line))
                {
                    lexAnalyzeShow_Token << line << endl;
                    lexAnalyzeShow_Type << typeToString[word.lexType] << endl;
                }
            }

            sort(res.begin(), res.end(), cmp);

            // 处理word的报错
            int wordType = dic["Word"];

            for (const auto &word : res)
            {
                if (word.lexType == wordType)
                {
                    wordErrorTypeSign = 1;
                    break;
                }
            }
            this->stLexAns(outFile, outFile2, bufLex, res);
            bufLex.clear(); // Buffer清除
        }
    }

    outFile2 << -1 << -1 << -1 << endl; // 栈底的-1

    // 提示打印
    if (ifCommentsError)
    {
        outFile << "【错误】多行注释词法分析错误！" << endl;
        Message += "【错误】多行注释词法分析错误！\n";
    }
    else if (wordErrorTypeSign)
    {
        outFile << "【错误】Token划分词法分析错误！" << endl;
        Message += "【错误】Token划分词法分析错误！\n";
    }

    // 文件关闭
    inFile.close();
    outFile.close();
    outFile2.close();
    lexAnalyzeShow_Token.close();
    lexAnalyzeShow_Type.close();

    // 结果返回
    if (ifCommentsError || wordErrorTypeSign)
        return 0;
    else
        return 1;
}
