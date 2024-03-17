#include "analyzer.h"

list<int> merge(list<int> &l1, list<int> &l2)
{
    list<int> ret;
    ret.assign(l1.begin(), l1.end());
    ret.splice(ret.end(), l2);
    return ret;
}

Symbol *Compile_Analyzer::popSymbol()
{
    Symbol *t = symbolStack.top();
    symbolStack.pop();
    stateStack.pop();
    return t;
}

void Compile_Analyzer::pushSymbol(Symbol *sym)
{
    symbolStack.push(sym);
}

Func *Compile_Analyzer::searchFunction(string ID)
{
    for (vector<Func>::iterator iter = funcTable.begin(); iter != funcTable.end(); iter++)
        if (iter->name == ID)
            return &(*iter);
    return NULL;
}

Var *Compile_Analyzer::searchVariable(string ID)
{
    for (vector<Var>::reverse_iterator iter = varTable.rbegin(); iter != varTable.rend(); iter++)
        if (iter->name == ID)
            return &(*iter);
    return NULL;
}

// @func : 输出状态栈和符号栈
void Compile_Analyzer::outputStack(ostream &fileout)
{
    int NUM_STATE = 8;
    int NUM_SYMBOL = 6;

    int cnt;

    // 状态栈
    cnt = 0;
    stack<int> state = this->stateStack;
    while (!state.empty())
    {
        fileout << state.top() << " ";
        state.pop();
        cnt++;
        if (cnt >= NUM_STATE)
            break;
    }
    fileout << endl;

    // 符号栈
    cnt = 0;
    stack<Symbol *> symbol = this->symbolStack;
    while (!symbol.empty())
    {
        fileout << symbol.top()->val << " ";
        symbol.pop();
        cnt++;
        if (cnt >= NUM_SYMBOL)
            break;
    }
    fileout << endl;
}

int Compile_Analyzer::regular_LR1(const char *AnalyzeProcess, const char *SynTaxTree)
{

    // 打开结果输出文件
    ofstream outFile(AnalyzeProcess, ios::out);
    if (!outFile.is_open())
    {
        cerr << "【错误】无法打开" << AnalyzeProcess << "文件！\n";
        Message += "【错误】无法打开";
        Message += AnalyzeProcess;
        Message += "文件！\n";
        return 0;
    }

    // 数据初始化
    stack<vector<int>> regularSet; // 归约记录

    // 状态栈与符号栈清空
    while (!stateStack.empty())
        stateStack.pop();
    while (!symbolStack.empty())
        symbolStack.pop();

    int nowLevel = 0;                        // 记录参数所在层数
    int stepsCount = 0;                      // 用于计数
    stateStack.push(START_STATE);            // 初始化状态
    symbolStack.push(new Symbol(true, "#")); // 栈底符#
    interCodeGenerator.errorMessage.clear(); // 清空错误信息
    bool sematicisFailed = false;            // 语义分析正误

    // 移进归约
    for (int wordsLocation = 0;;)
    {
        //------------------------获取当前状态和当前符号，下一状态------------------------
        int currentState = stateStack.top();
        int str = tokens[wordsLocation].id;
        string tokenType = tokens[wordsLocation].tokenType;
        string tokenValue = tokens[wordsLocation].tokenValue;
        if (str != 0)
            str += 1;

        int nextState = tableLR1[currentState][str];
        int judge = shiftReduceTable[currentState][str];

        //------------------------下一个符号的转化------------------------
        Symbol *nextSymbol;
        if (tokenType == "Identifier")
            nextSymbol = new Identifier(Symbol{true, "Identifier"}, tokenValue);
        else if (tokenType == "Integer")
            nextSymbol = new Num(Symbol{true, "Integer"}, tokenValue);
        else
            nextSymbol = new Symbol(true, tokenType);

        //------------------------根据LR1表进行跳转------------------------
        // 空白错误态
        if (nextState == TBSTATE_NONE)
        {
            cerr << "【错误】在生成LR1的ActionGOTO表时候有错误！\n";
            Message += ">>> 【错误】在生成LR1的ActionGOTO表时候有错误！\n";
            return RETURN_ERROR;
        }
        // 接受态
        else if (nextState == TBSTATE_ACC)
        {
            // 归约过程记录
            regularSet.push(production[0]);

            outFile << "ACC!" << endl;

            generateSyntaxTree(regularSet, SynTaxTree);

            // "P ::= N declare_list" -- 回填最终main函数
            Func *f = searchFunction("main");
            popSymbol();
            N *n = (N *)popSymbol();
            interCodeGenerator.BackPatch(n->nextList, f->enterPoint);

            return RETURN_FINE;
        }
        // 移进
        else if (judge == TBSTATE_SHIFT)
        {
            // 入栈新元素
            symbolStack.push(nextSymbol);
            stateStack.push(nextState);
            // 内容打印
            outputStack(outFile);
            outFile << "∵Action[" << abs(currentState) << "," << symbolStr[str] << "]=" << abs(nextState);
            outFile << "，∴" << abs(nextState) << "入栈!" << endl;
            // 输入串位置后移
            wordsLocation++;
        }
        // 归约
        else if (judge == TBSTATE_REGULAR)
        {
            vector<int> reductPro = production[abs(nextState)]; // 获取产生式
            int popSymNum = (int)(reductPro.size()) - 1;        // 去掉产生式左边

            switch (abs(nextState))
            {
            case 3: // declare ::= int ID M A function_declare
            {
                FunctionDeclaration *function_declare = (FunctionDeclaration *)popSymbol();
                Symbol *A = popSymbol();
                M *m = (M *)popSymbol();
                Identifier *ID = (Identifier *)popSymbol();
                Symbol *_int = popSymbol();
                funcTable.push_back(Func{ID->name, INTType, function_declare->parameterTypes, m->quad});
                pushSymbol(new Symbol(false, "declare"));
                break;
            }
            case 4: // declare ::= int ID var_declare
            {
                Symbol *var_declare = popSymbol();
                Identifier *ID = (Identifier *)popSymbol();
                Symbol *_int = popSymbol();
                varTable.push_back(Var{ID->name, INTType, nowLevel});
                pushSymbol(new Symbol(false, "declare"));
                break;
            }
            case 5: // declare ::= void ID M A function_declare
            {
                FunctionDeclaration *function_declare = (FunctionDeclaration *)popSymbol();
                Symbol *A = popSymbol();
                M *m = (M *)popSymbol();
                Identifier *ID = (Identifier *)popSymbol();
                Symbol *_void = popSymbol();
                funcTable.push_back(Func{ID->name, VOIDType, function_declare->parameterTypes, m->quad});
                pushSymbol(new Symbol(false, "declare"));
                break;
            }
            case 6: // A ::=
            {
                nowLevel++;
                pushSymbol(new Symbol(false, "A"));
                break;
            }
            case 8: // function_declare ::= ( parameter ) sentence_block
            {
                StatementBlock *sentence_block = (StatementBlock *)popSymbol();
                Symbol *rparen = popSymbol();
                Parameter *paramter = (Parameter *)popSymbol();
                Symbol *lparen = popSymbol();
                FunctionDeclaration *function_declare = new FunctionDeclaration(*(new Symbol(false, "function_declare")));
                function_declare->parameterTypes.assign(paramter->parameterTypes.begin(), paramter->parameterTypes.end());
                pushSymbol(function_declare);
                break;
            }
            case 9: // parameter :: = parameter_list
            {
                ParameterList *parameter_list = (ParameterList *)popSymbol();
                Parameter *parameter = new Parameter(*(new Symbol(false, "parameter")));
                parameter->parameterTypes.assign(parameter_list->parameterTypes.begin(), parameter_list->parameterTypes.end());
                pushSymbol(parameter);
                break;
            }
            case 10: // parameter ::= void
            {
                Symbol *_void = popSymbol();
                Parameter *parameter = new Parameter(*(new Symbol(false, "parameter")));
                pushSymbol(parameter);
                break;
            }
            case 11: // parameter_list ::= param
            {
                Symbol *param = popSymbol();
                ParameterList *parameter_list = new ParameterList(*(new Symbol(false, "parameter_list")));
                parameter_list->parameterTypes.push_back(INTType);
                pushSymbol(parameter_list);
                break;
            }
            case 12: // parameter_list1 ::= param , parameter_list2
            {
                ParameterList *parameter_list2 = (ParameterList *)popSymbol();
                Symbol *comma = popSymbol();
                Symbol *param = popSymbol();
                ParameterList *parameter_list1 = new ParameterList(*(new Symbol(false, "parameter_list")));
                parameter_list2->parameterTypes.push_front(INTType);
                parameter_list1->parameterTypes.assign(parameter_list2->parameterTypes.begin(), parameter_list2->parameterTypes.end());
                pushSymbol(parameter_list1);
                break;
            }
            case 13: // param ::= int ID
            {
                Identifier *ID = (Identifier *)popSymbol();
                Symbol *_int = popSymbol();
                varTable.push_back(Var{ID->name, INTType, nowLevel});
                interCodeGenerator.Emit("get", "_", "_", ID->name); // EMIT
                pushSymbol(new Symbol(false, "param"));
                break;
            }
            case 14: // sentence_block ::= { inner_declare sentence_list }
            {
                Symbol *rbrace = popSymbol();
                StatementList *sentence_list = (StatementList *)popSymbol();
                Symbol *inner_declare = popSymbol();
                Symbol *lbrace = popSymbol();
                StatementBlock *sentence_block = new StatementBlock(*(new Symbol(false, "sentence_block")));
                sentence_block->nextList = sentence_list->nextList;
                nowLevel--;
                int popNum = 0;
                for (vector<Var>::reverse_iterator riter = varTable.rbegin(); riter != varTable.rend(); riter++)
                {
                    if (riter->level > nowLevel)
                        popNum++;
                    else
                        break;
                }
                for (int i = 0; i < popNum; i++)
                    varTable.pop_back();
                pushSymbol(sentence_block);
                break;
            }
            case 17: // inner_var_declare ::= int ID
            {
                Identifier *ID = (Identifier *)popSymbol();
                Symbol *_int = popSymbol();
                pushSymbol(new Symbol(false, "inner_var_declare"));
                varTable.push_back(Var{ID->name, INTType, nowLevel});
                break;
            }
            case 18: // sentence_list ::= sentence M sentence_list
            {
                StatementList *sentence_list2 = (StatementList *)popSymbol();
                M *m = (M *)popSymbol();
                Sentence *sentence = (Sentence *)popSymbol();
                StatementList *sentence_list1 = new StatementList(*(new Symbol(false, "sentence_list")));
                sentence_list1->nextList = sentence_list2->nextList;
                interCodeGenerator.BackPatch(sentence->nextList, m->quad);
                pushSymbol(sentence_list1);
                break;
            }
            case 19: // sentence_list ::= sentence
            {
                Sentence *sentence = (Sentence *)popSymbol();
                StatementList *sentence_list = new StatementList(*(new Symbol(false, "sentence_list")));
                sentence_list->nextList = sentence->nextList;
                pushSymbol(sentence_list);
                break;
            }
            case 20: // sentence ::= if_sentence
            {
                IfSentence *if_sentence = (IfSentence *)popSymbol();
                Sentence *sentence = new Sentence(*(new Symbol(false, "sentence")));
                sentence->nextList = if_sentence->nextList;
                pushSymbol(sentence);
                break;
            }
            case 21: // sentence ::= while_sentence
            {
                WhileSentence *while_sentence = (WhileSentence *)popSymbol();
                Sentence *sentence = new Sentence(*(new Symbol(false, "sentence")));
                sentence->nextList = while_sentence->nextList;
                pushSymbol(sentence);
                break;
            }
            case 22: // sentence ::= return_sentence
            {
                Symbol *return_sentence = popSymbol();
                Sentence *sentence = new Sentence(*(new Symbol(false, "sentence")));
                pushSymbol(sentence);
                break;
            }
            case 23: // sentence ::= assign_sentence
            {
                Symbol *assign_sentence = popSymbol();
                Sentence *sentence = new Sentence(*(new Symbol(false, "sentence")));
                pushSymbol(sentence);
                break;
            }
            case 24: // assign_sentence ::= ID = expression ;
            {
                Symbol *comma = popSymbol();
                Expression *expression = (Expression *)popSymbol();
                Symbol *assign = popSymbol();
                Identifier *ID = (Identifier *)popSymbol();
                if (searchVariable(ID->name) == NULL)
                {
                    cerr << (string("语法错误变量" + ID->name + "未声明！")) << endl;
                    Message += string(">>> 【错误】语法错误变量" + ID->name + "未声明！\n");
                    interCodeGenerator.errorMessage.push_back("语法错误变量" + ID->name + "未声明！");
                    sematicisFailed = true;
                }
                Symbol *assign_sentence = new Symbol(false, "assign_sentence");
                interCodeGenerator.Emit("=", expression->name, "_", ID->name); // EMIT
                pushSymbol(assign_sentence);
                break;
            }
            case 25: // return_sentence ::= return ;
            {
                Symbol *comma = popSymbol();
                Symbol *_return = popSymbol();
                interCodeGenerator.Emit("return", "_", "_", "_"); // EMIT
                pushSymbol(new Symbol(false, "return_sentence"));
                break;
            }
            case 26: // return_sentence ::= return expression ;
            {
                Symbol *comma = popSymbol();
                Expression *expression = (Expression *)popSymbol();
                Symbol *_return = popSymbol();
                interCodeGenerator.Emit("return", expression->name, "_", "_"); // EMIT
                pushSymbol(new Symbol(false, "return_sentence"));
                break;
            }
            case 27: // while_sentence ::= while M ( expression ) A sentence_block
            {
                StatementBlock *sentence_block = (StatementBlock *)popSymbol();
                Symbol *A = popSymbol();
                Symbol *rparen = popSymbol();
                Expression *expression = (Expression *)popSymbol();
                Symbol *lparen = popSymbol();
                M *m = (M *)popSymbol();
                Symbol *_while = popSymbol();
                WhileSentence *while_sentence = new WhileSentence(*(new Symbol(false, "while_sentence")));
                interCodeGenerator.BackPatch(sentence_block->nextList, m->quad); // 回填M四元式的地址
                while_sentence->nextList = expression->falseList;                //
                interCodeGenerator.Emit("j", "_", "_", to_string(m->quad));      // EMIT
                pushSymbol(while_sentence);
                break;
            }
            case 28: // if_sentence ::= if ( expression ) A sentence_block
            {
                StatementBlock *sentence_block = (StatementBlock *)popSymbol();
                Symbol *A = popSymbol();
                Symbol *rparen = popSymbol();
                Expression *expression = (Expression *)popSymbol();
                Symbol *lparen = popSymbol();
                Symbol *_if = popSymbol();
                IfSentence *if_sentence = new IfSentence(*(new Symbol(false, "if_sentence")));
                expression->falseList.splice(expression->falseList.begin(), sentence_block->nextList);
                if_sentence->nextList = expression->falseList;
                pushSymbol(if_sentence);
                break;
            }
            case 29: // if_sentence ::= if ( expression ) A1 sentence_block1 N else M A2 sentence_block2
            {
                StatementBlock *sentence_block2 = (StatementBlock *)popSymbol();
                Symbol *A2 = popSymbol();
                M *m = (M *)popSymbol();
                Symbol *_else = popSymbol();
                N *n = (N *)popSymbol();
                StatementBlock *sentence_block1 = (StatementBlock *)popSymbol();
                Symbol *A1 = popSymbol();
                Symbol *rparen = popSymbol();
                Expression *expression = (Expression *)popSymbol();
                Symbol *lparen = popSymbol();
                Symbol *_if = popSymbol();
                IfSentence *if_sentence = new IfSentence(*(new Symbol(false, "if_sentence")));
                interCodeGenerator.BackPatch(expression->falseList, m->quad);
                if_sentence->nextList = merge(sentence_block1->nextList, sentence_block2->nextList);
                if_sentence->nextList = merge(if_sentence->nextList, n->nextList);
                pushSymbol(if_sentence);
                break;
            }
            case 30: // N ::=
            {
                N *n = new N(*(new Symbol(false, "N")));
                n->nextList.push_back(interCodeGenerator.NextQuaternaryIndex());
                interCodeGenerator.Emit("j", "_", "_", "-1"); // EMIT
                pushSymbol(n);
                break;
            }
            case 31: // M ::=
            {
                M *m = new M(*(new Symbol(false, "M")));
                m->quad = interCodeGenerator.NextQuaternaryIndex();
                pushSymbol(m);
                break;
            }
            case 32: // expression ::= add_expression
            {
                AddExpression *add_expression = (AddExpression *)popSymbol();
                Expression *expression = new Expression(*(new Symbol(false, "expression")));
                expression->name = add_expression->name;
                pushSymbol(expression);
                break;
            }
            case 33: // expression ::= add_expression1 > add_expression2
            {
                AddExpression *add_expression2 = (AddExpression *)popSymbol();
                Symbol *gt = popSymbol();
                AddExpression *add_expression1 = (AddExpression *)popSymbol();
                Expression *expression = new Expression(*(new Symbol(false, "expression")));
                expression->falseList.push_back(interCodeGenerator.NextQuaternaryIndex());
                interCodeGenerator.Emit("j<=", add_expression1->name, add_expression2->name, "-1"); // EMIT
                pushSymbol(expression);
                break;
            }
            case 34: // expression ::= add_expression1 < add_expression2
            {
                AddExpression *add_expression2 = (AddExpression *)popSymbol();
                Symbol *lt = popSymbol();
                AddExpression *add_expression1 = (AddExpression *)popSymbol();
                Expression *expression = new Expression(*(new Symbol(false, "expression")));
                expression->falseList.push_back(interCodeGenerator.NextQuaternaryIndex());
                interCodeGenerator.Emit("j>=", add_expression1->name, add_expression2->name, "-1"); // EMIT
                pushSymbol(expression);
                break;
            }
            case 35: // expression ::= add_expression1 == add_expression2
            {
                AddExpression *add_expression2 = (AddExpression *)popSymbol();
                Symbol *eq = popSymbol();
                AddExpression *add_expression1 = (AddExpression *)popSymbol();
                Expression *expression = new Expression(*(new Symbol(false, "expression")));
                expression->falseList.push_back(interCodeGenerator.NextQuaternaryIndex());
                interCodeGenerator.Emit("j!=", add_expression1->name, add_expression2->name, "-1"); // EMIT
                pushSymbol(expression);
                break;
            }
            case 36: // expression ::= add_expression1 >= add_expression2
            {
                AddExpression *add_expression2 = (AddExpression *)popSymbol();
                Symbol *get = popSymbol();
                AddExpression *add_expression1 = (AddExpression *)popSymbol();
                Expression *expression = new Expression(*(new Symbol(false, "expression")));
                expression->falseList.push_back(interCodeGenerator.NextQuaternaryIndex());
                interCodeGenerator.Emit("j<", add_expression1->name, add_expression2->name, "-1"); // EMIT
                pushSymbol(expression);
                break;
            }
            case 37: // expression ::= add_expression1 <= add_expression2
            {
                AddExpression *add_expression2 = (AddExpression *)popSymbol();
                Symbol *let = popSymbol();
                AddExpression *add_expression1 = (AddExpression *)popSymbol();
                Expression *expression = new Expression(*(new Symbol(false, "expression")));
                expression->falseList.push_back(interCodeGenerator.NextQuaternaryIndex());
                interCodeGenerator.Emit("j>", add_expression1->name, add_expression2->name, "-1"); // EMIT
                pushSymbol(expression);
                break;
            }
            case 38: // expression ::= add_expression1 != add_expression2
            {
                AddExpression *add_expression2 = (AddExpression *)popSymbol();
                Symbol *neq = popSymbol();
                AddExpression *add_expression1 = (AddExpression *)popSymbol();
                Expression *expression = new Expression(*(new Symbol(false, "expression")));
                expression->falseList.push_back(interCodeGenerator.NextQuaternaryIndex());
                interCodeGenerator.Emit("j==", add_expression1->name, add_expression2->name, "-1"); // EMIT
                pushSymbol(expression);
                break;
            }
            case 39: // add_expression ::= item
            {
                Nomial *item = (Nomial *)popSymbol();
                AddExpression *add_expression = new AddExpression(*(new Symbol(false, "add_expression")));
                add_expression->name = item->name;
                pushSymbol(add_expression);
                break;
            }
            case 40: // add_expression1 ::= item + add_expression2
            {
                AddExpression *add_expression2 = (AddExpression *)popSymbol();
                Symbol *add = popSymbol();
                Nomial *item = (Nomial *)popSymbol();
                AddExpression *add_expression1 = new AddExpression(*(new Symbol(false, "add_expression")));
                add_expression1->name = labelGenerator.new_label();
                interCodeGenerator.Emit("+", item->name, add_expression2->name, add_expression1->name); // EMIT
                pushSymbol(add_expression1);
                break;
            }
            case 41: // add_expression ::= item - add_expression
            {
                AddExpression *add_expression2 = (AddExpression *)popSymbol();
                Symbol *sub = popSymbol();
                Nomial *item = (Nomial *)popSymbol();
                AddExpression *add_expression1 = new AddExpression(*(new Symbol(false, "add_expression")));
                add_expression1->name = labelGenerator.new_label();
                interCodeGenerator.Emit("-", item->name, add_expression2->name, add_expression1->name); // EMIT
                pushSymbol(add_expression1);
                break;
            }
            case 42: // item ::= factor
            {
                Factor *factor = (Factor *)popSymbol();
                Nomial *item = new Nomial(*(new Symbol(false, "item")));
                item->name = factor->name;
                pushSymbol(item);
                break;
            }
            case 43: // item1 ::= factor * item2
            {
                Nomial *item2 = (Nomial *)popSymbol();
                Symbol *mul = popSymbol();
                Factor *factor = (Factor *)popSymbol();
                Nomial *item1 = new Nomial(*(new Symbol(false, "item")));
                item1->name = labelGenerator.new_label();
                interCodeGenerator.Emit("*", factor->name, item2->name, item1->name); // EMIT
                pushSymbol(item1);
                break;
            }
            case 44: // item1 ::= factor / item2
            {
                Nomial *item2 = (Nomial *)popSymbol();
                Symbol *div = popSymbol();
                Factor *factor = (Factor *)popSymbol();
                Nomial *item1 = new Nomial(*(new Symbol(false, "item")));
                item1->name = labelGenerator.new_label();
                interCodeGenerator.Emit("/", factor->name, item2->name, item1->name); // EMIT
                pushSymbol(item1);
                break;
            }
            case 45: // factor ::= NUM
            {
                Num *num = (Num *)popSymbol();
                Factor *factor = new Factor(*(new Symbol(false, "factor")));
                factor->name = num->number;
                pushSymbol(factor);
                break;
            }
            case 46: // factor ::= ( expression )
            {
                Symbol *rparen = popSymbol();
                Expression *expression = (Expression *)popSymbol();
                Symbol *lparen = popSymbol();
                Factor *factor = new Factor(*(new Symbol(false, "factor")));
                factor->name = expression->name;
                pushSymbol(factor);
                break;
            }
            case 47: // factor ::= ID ( argument_list )
            {
                Symbol *rparen = popSymbol();
                ArgumentList *argument_list = (ArgumentList *)popSymbol();
                Symbol *lparen = popSymbol();
                Identifier *ID = (Identifier *)popSymbol();
                Factor *factor = new Factor(*(new Symbol(false, "factor")));
                Func *f = searchFunction(ID->name);
                if (!f)
                {
                    cerr << (string("语法错误未声明的函数" + ID->name)) << "！\n";
                    Message += string(">>> 【错误】语法错误未声明的函数" + ID->name + "！\n");
                    interCodeGenerator.errorMessage.push_back("语法错误未声明的函数" + ID->name);
                    sematicisFailed = true;
                    Factor *factor = new Factor(*(new Symbol(false, "factor")));
                    pushSymbol(factor);
                }
                else if (argument_list->alist.size() != f->paramTypes.size())
                {
                    cerr << (string("语法错误函数" + ID->name + "参数不匹配！")) << "\n";
                    Message += string(">>> 【错误】语法错误函数" + ID->name + "参数不匹配！\n");
                    interCodeGenerator.errorMessage.push_back("语法错误函数" + ID->name + "参数不匹配");
                    sematicisFailed = true;
                    Factor *factor = new Factor(*(new Symbol(false, "factor")));
                    pushSymbol(factor);
                }
                else
                {
                    for (list<string>::iterator iter = argument_list->alist.begin(); iter != argument_list->alist.end(); iter++)
                    {
                        interCodeGenerator.Emit("par", *iter, "_", "_");
                    }
                    factor->name = labelGenerator.new_label();
                    interCodeGenerator.Emit("call", ID->name, "_", "_");              // EMIT
                    interCodeGenerator.Emit("=", "@RETURN_PLACE", "_", factor->name); // EMIT

                    pushSymbol(factor);
                }
                break;
            }
            case 48: // factor ::= ID
            {
                Identifier *ID = (Identifier *)popSymbol();
                if (searchVariable(ID->name) == NULL)
                {
                    cerr << (string("语法错误变量" + ID->name + "未声明！")) << "\n";
                    Message += string(">>> 【错误】语法错误变量" + ID->name + "未声明！\n");
                    interCodeGenerator.errorMessage.push_back("语法错误变量" + ID->name + "未声明！");
                    sematicisFailed = true;
                }
                Factor *factor = new Factor(*(new Symbol(false, "factor")));
                factor->name = ID->name;
                pushSymbol(factor);
                break;
            }
            case 49: // argument_list ::=
            {
                ArgumentList *argument_list = new ArgumentList(*(new Symbol(false, "argument_list")));
                pushSymbol(argument_list);
                break;
            }
            case 50: // argument_list ::= expression
            {
                Expression *expression = (Expression *)popSymbol();
                ArgumentList *argument_list = new ArgumentList(*(new Symbol(false, "argument_list")));
                argument_list->alist.push_back(expression->name);
                pushSymbol(argument_list);
                break;
            }
            case 51: // argument_list1 ::= expression , argument_list2
            {
                ArgumentList *argument_list2 = (ArgumentList *)popSymbol();
                Symbol *comma = popSymbol();
                Expression *expression = (Expression *)popSymbol();
                ArgumentList *argument_list1 = new ArgumentList(*(new Symbol(false, "argument_list")));
                argument_list2->alist.push_front(expression->name);
                argument_list1->alist.assign(argument_list2->alist.begin(), argument_list2->alist.end());
                pushSymbol(argument_list1);
                break;
            }
            default:
                for (int i = 0; i < popSymNum; i++)
                {
                    popSymbol();
                }
                pushSymbol(new Symbol(false, string(VNNAMES[reductPro[0] - VTSIZE - 1]))); //
                break;
            }

            // 归约过程记录
            regularSet.push(production[abs(nextState)]);

            // 归约打印
            int cur = stateStack.top();

            outputStack(outFile);
            outFile << "∵有“" << symbolStr[production[abs(nextState)][0]] << "->";
            for (int ii = 1; ii <= popSymNum; ii++)
            {
                outFile << symbolStr[production[abs(nextState)][ii]];
                if (ii != popSymNum)
                {
                    outFile << ', ';
                }
            }
            outFile << "”";
            outFile << "，∴GOTO[" << cur << "," << symbolStr[str] << "]=" << tableLR1[cur][production[abs(nextState)][0]];
            outFile << "入栈！" << endl;
            stateStack.push(abs(tableLR1[cur][production[abs(nextState)][0]]));
        }
        // 出错
        else
        {
            cerr << "【错误】在生成LR1的ActionGOTO表时候有错误！\n";
            Message += ">>> 【错误】在生成LR1的ActionGOTO表时候有错误！\n";
            return RETURN_ERROR;
        }
    }

    return RETURN_FINE;
}
