//@func   :  qt页面总控制程序

#include "myCompiler.h"

using namespace std;

string Message;

//---------------------------------------------------------

//@func : 显示Message
void myCompiler::message_show(void)
{
    // QString text = QString::fromLocal8Bit(Message.c_str());
    QString text = Message.c_str();
    qDebug() << text;
    this->ui.textBrowser_log->append(text);
}

//@func : 计算一个文件有多少行
int count_lines(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "【警告】无法打开文件" << filename << std::endl;
        return -1;
    }

    int lineCount = 0;
    std::string line;
    while (std::getline(file, line))
        lineCount++;

    file.close();
    return lineCount;
}

//@func : 递归删除文件夹
int deleteFolderContents(const QString &folderPath)
{
    QDir folderDir(folderPath);

    if (!folderDir.exists())
    {
        qDebug() << "Folder does not exist.";
        return RETURN_FINE;
    }

    QFileInfoList fileList = folderDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    foreach (const QFileInfo &fileInfo, fileList)
    {
        if (fileInfo.isDir())
        {
            // 递归删除子文件夹
            deleteFolderContents(fileInfo.absoluteFilePath());
            folderDir.rmdir(fileInfo.absoluteFilePath());
        }
        else
        {
            // 删除文件
            folderDir.remove(fileInfo.absoluteFilePath());
        }
    }

    return RETURN_FINE;
}

//---------------------------------------------------------

//@func : 保存编码区内容
void myCompiler::save_textEdit(QString &filePath, QTextEdit *textEdit)
{
    // 获取 QTextEdit 的文本内容
    QString text = textEdit->toPlainText();

    // 将文本内容保存到文件中
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << text;
        file.close();
    }
    else
        QMessageBox::warning(this, tr("Warning"), tr("【警告】无法保存文件"));
}

//@func : 添加一张文件编辑页面
void myCompiler::addCode(const QString &filePath, bool isUnnamed)
{
    QFileInfo fileInfo(filePath);
    QFile file(filePath);

    if (isUnnamed)
    {
        if (!file.open(QFile::ReadOnly | QFile::Append))
        {
            QMessageBox::warning(this, tr("Warning"), tr("【警告】无法打开文件！"));
            return;
        }
    }
    else
    {
        if (!file.open(QFile::ReadOnly))
        {
            QMessageBox::warning(this, tr("Warning"), tr("【警告】无法打开文件！"));
            return;
        }
    }

    // 添加代码标签页
    int index = this->ui.tabWidget_code->count();
    CodeWidgetTab *tabCode = new CodeWidgetTab(this);
    if (isUnnamed)
        tabCode->isUnnamed = true;
    tabCode->textEdit->append(file.readAll());
    tabCode->textEdit->show();
    tabCode->textEdit->setFocusPolicy(Qt::StrongFocus);
    tabTextEditMap[index] = tabCode;
    QFont consolasFont("Consolas");
    tabCode->textEdit->setFont(consolasFont);

    this->ui.tabWidget_code->addTab(tabCode, fileInfo.fileName());
    this->ui.tabWidget_code->setTabToolTip(index, fileInfo.absoluteFilePath());

    this->fileNum++;

    connect(tabCode->textEdit, &ZoomableTextEdit::textChanged, this, &myCompiler::modify_triggered);

    return;
}

//@func : 构造函数
myCompiler::myCompiler(QWidget *parent) : QMainWindow(parent)
{

    // 初始化UI
    ui.setupUi(this);

    //
    this->fileNum = 0;

    this->ui.tabWidget_code->clear();               //  清除原有界面
    this->ui.tabWidget_code->setMovable(true);      //  设置为可移动
    this->ui.tabWidget_code->setTabsClosable(true); //	在选项卡上添加关闭按钮
    this->ui.tabWidget_code->usesScrollButtons();   //	选项卡太多滚动
                                                    //    this->ui.tabWidget_code->setGeometry(QRect(2, 2, 200, 294));
    this->ui.tabWidget_code->setStyleSheet("background-color: #F1FBFF;");
    QString tableStyle = "QTableWidget { border: 1px solid gray; border-radius: 5px; }"
                         "QTableWidget::item { padding: 5px; }"
                         "QTableWidget::item:selected { background-color: lightgray; }";
    this->ui.tabWidget_show->setStyleSheet(tableStyle);
    this->ui.tabWidget_show->setStyleSheet("background-color: #F1FBFF;");
    this->ui.textBrowser_log->setStyleSheet("background-color: #F1FBFF;");

    this->ui.tabWidget_show->clear();
    this->ui.tabWidget_show->setMovable(true);
    this->ui.tabWidget_show->usesScrollButtons();
    this->ui.menuBar->setStyleSheet("QMenuBar { color: #1661ab; } QMenuBar::item:selected { background-color: lightblue; }");

    connect(this->ui.actionNew, &QAction::triggered, this, &myCompiler::actionNew_triggered);
    connect(this->ui.actionOpen, &QAction::triggered, this, &myCompiler::actionOpen_triggered);
    connect(this->ui.actionSave, &QAction::triggered, this, &myCompiler::actionSave_triggered);
    connect(this->ui.tabWidget_code, &QTabWidget::tabCloseRequested, this, &myCompiler::closeRequest_triggered);

    // 初始化
    this->isLex = false;
    this->isSyn = false;
    this->isSem = false;
    this->isItermediate = false;
    this->isObject = false;

    // 创建tmp临时文件夹
    if (0 != access(folderPath.c_str(), 0))
        mkdir(folderPath.c_str()); // 返回 0 创建成功 | -1 表示失败
    else
        deleteFolderContents(QString::fromStdString(folderPath));

    // 创建src资源文件夹
    if (0 != access(srcPath.c_str(), 0))
        mkdir(srcPath.c_str()); // 返回 0 创建成功 | -1 表示失败
    else
        deleteFolderContents(QString::fromStdString(srcPath));

    // 创建files文件夹
    if (0 != access(defaultCodeSavePath.c_str(), 0))
        mkdir(defaultCodeSavePath.c_str()); // 返回 0 创建成功 | -1 表示失败
    else
        deleteFolderContents(QString::fromStdString(defaultCodeSavePath));

    // 是否为LR1文法
    if (this->anaylzer.initForAll() == 0)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】不遵守LR1文法！"));
        this->message_show();
        this->isLR1 = false;
    }
    else
        this->isLR1 = true;

    connect(this->ui.actionLexical_Analyze, &QAction::triggered, this, &myCompiler::actionLexicalAnalyze_triggered);
    connect(this->ui.actionSyntactic_Analyze, &QAction::triggered, this, &myCompiler::actionSyntacticAnalyze_triggered);
    connect(this->ui.actionSemantic_Analyze, &QAction::triggered, this, &myCompiler::actionSemanticAnalyze_triggered);
    connect(this->ui.actionIntermediate_Code, &QAction::triggered, this, &myCompiler::actionIntermediateCode_triggered);
    connect(this->ui.actionObject_Code, &QAction::triggered, this, &myCompiler::actionObjectCode_triggered);
    connect(this->ui.actionshowRes, &QAction::triggered, this, [=]()
            { this->ui.dockWidget_show->setHidden(false); });
    connect(this->ui.actionshowLog, &QAction::triggered, this, [=]()
            { this->ui.dockWidget_log->setHidden(false); });
    connect(this->ui.actionTree, &QAction::triggered, this, &myCompiler::actionTree_triggered);
    connect(this->ui.pushButton_clear, &QPushButton::clicked, this, [=]()
            { this->ui.textBrowser_log->clear(); });
    connect(this->ui.actionSave_Object_Code, &QAction::triggered, this, &myCompiler::actionSave_ObjectCode_triggered);
    connect(this->ui.actionSave_Object_Code_2, &QAction::triggered, this, &myCompiler::actionSave_ObjectCode_triggered);
    connect(this->ui.actionLexAnalyzer, &QAction::triggered, this, &myCompiler::actionLexAnalyzer_triggered);
    connect(this->ui.actionToken_Type, &QAction::triggered, this, &myCompiler::actionToken_Type_triggered);
    connect(this->ui.actionitemSet, &QAction::triggered, this, &myCompiler::actionitemSet_triggered);
    connect(this->ui.actionReductionProcess, &QAction::triggered, this, &myCompiler::actionReductionProcess_triggered);
    connect(this->ui.actionActionGOTO, &QAction::triggered, this, &myCompiler::actionActionGOTO_triggered);
    connect(this->ui.actioninterCode, &QAction::triggered, this, &myCompiler::actioninterCode_triggered);
    connect(this->ui.actionfuncBlock, &QAction::triggered, this, &myCompiler::actionfuncBlock_triggered);
    connect(this->ui.actionwaitUseTable, &QAction::triggered, this, &myCompiler::actionwaitUseTable_triggered);
    connect(this->ui.actiongenCode, &QAction::triggered, this, &myCompiler::actiongenCode_triggered);
    connect(this->ui.actiongrammar, &QAction::triggered, this, &myCompiler::actiongrammar_triggered);
}

//@func : 析构函数
myCompiler::~myCompiler()
{
    ;
}

bool myCompiler::ifFileAvailable(void)
{
    int index;

    // 能否进行
    if (this->ui.tabWidget_code->count() == 0)
    {
        QMessageBox::warning(this, tr("Warning"), tr("【警告】没有需要处理的文件！"));
        return false;
    }
    index = this->ui.tabWidget_code->currentIndex();
    if (this->tabTextEditMap[index]->textEdit->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, tr("Warning"), tr("【警告】没有需要处理的文件！"));
        return false;
    }
    else if (this->tabTextEditMap[index]->isModify)
    {
        QMessageBox::warning(this, tr("Warning"), tr("【警告】请先保存您的修改！"));
        return false;
    }

    // 获取 QTextEdit 的文本内容
    QString text = this->tabTextEditMap[index]->textEdit->toPlainText();

    // 将文本内容保存到文件中
    QFile file(QString::fromStdString(folderPath + input));
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << text;
        file.close();
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"), tr("【错误】无法保存文件！"));
    }

    return true;
}

//@func :
void myCompiler::modify_triggered(void)
{
    int index = this->ui.tabWidget_code->currentIndex();
    if (!this->tabTextEditMap[index]->isModify)
    {
        this->tabTextEditMap[index]->isModify = true;
        QString title = this->ui.tabWidget_code->tabText(index);
        if (!title.endsWith("*"))
            this->ui.tabWidget_code->setTabText(index, title + "*");

        QFont consolasFont("Consolas");
        this->tabTextEditMap[index]->textEdit->setFont(consolasFont);
    }
    return;
}

//@func :
void myCompiler::actionSave_triggered(void)
{
    if (this->fileNum != 0)
    {
        int index = this->ui.tabWidget_code->currentIndex();
        // 若为新建文件
        if (this->tabTextEditMap[index]->isUnnamed)
        {
            QString defaultPath = QString::fromStdString(defaultCodeSavePath + defaultCodeSaveFile);
            QString selectedFilter;
            QString filePath = QFileDialog::getSaveFileName(this, "file saving", defaultPath, "c file (*.c);;all file (*)", &selectedFilter);
            if (!filePath.isEmpty())
            {
                // save
                save_textEdit(filePath, this->tabTextEditMap[index]->textEdit);
                // change name
                QFileInfo fileInfo(filePath);
                QString fileName = fileInfo.fileName();
                this->ui.tabWidget_code->setTabText(index, fileName);
                this->ui.tabWidget_code->setTabToolTip(index, filePath);
                // change symbol
                this->tabTextEditMap[index]->isModify = false;
                this->tabTextEditMap[index]->isUnnamed = false;
            }
        }
        // 若非新建文件
        else
        {
            // save
            QString toolTip = this->ui.tabWidget_code->tabToolTip(index);
            save_textEdit(toolTip, this->tabTextEditMap[index]->textEdit);
            // show
            QString title = this->ui.tabWidget_code->tabText(index);
            if (title.contains("*"))
            {
                title.remove("*");
                this->ui.tabWidget_code->setTabText(index, title);
            }
            this->tabTextEditMap[index]->isModify = false;
        }
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"), tr("【警告】没有文件需要保存！"));
    }
}

//@func : 打开文件
void myCompiler::actionOpen_triggered(void)
{
    QString path = QFileDialog::getOpenFileName(this, "open", "./", "TXT(*.txt *.dat *.c *.cpp);;ALL(*)");
    if (path.isEmpty() == false)
        addCode(path);
}

//@func : 新建文件
void myCompiler::actionNew_triggered(void)
{
    // 初始化
    int num = 1;
    bool isExited = false;

    // 遍历所有文件观察是否有重名
    QDir folderDir(QString::fromStdString(defaultCodeSavePath));
    QFileInfoList fileList = folderDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    QString name;
    while (true)
    {
        name = QString::fromStdString(defaultCodeSaveFile) + QString::fromStdString(to_string(num)) + ".c";
        foreach (const QFileInfo &fileInfo, fileList)
            if (fileInfo.fileName() == name)
                isExited = true;
        if (isExited)
        {
            num++;
            isExited = false;
            continue;
        }
        else
            break;
    }

    // 添加
    QString filePath = QString::fromStdString(defaultCodeSavePath) + name;
    this->addCode(filePath, true);

    return;
}

//@func : 关闭文件
void myCompiler::closeRequest_triggered(int index)
{
    // 提示保存
    if (this->tabTextEditMap[index]->isModify)
    {
        QMessageBox::StandardButton result = QMessageBox::question(this, "Notice", "Your changes are not saved.\nWould you like to save?", QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes)
        {
            this->actionSave_triggered();
            return;
        }
    }

    // 删除
    if (this->tabTextEditMap[index]->isUnnamed)
    {
        QString filePath = this->ui.tabWidget_code->tabToolTip(index);
        QFile file(filePath);
        if (file.exists())
            file.remove();
    }
    // 删除
    this->ui.tabWidget_code->removeTab(index);
    delete this->tabTextEditMap[index];
    // map更新
    for (int i = index; i < this->fileNum - 1; i++)
        this->tabTextEditMap[i] = this->tabTextEditMap[i + 1];
    this->fileNum--;
}

vector<string> splitString(const std::string &str, char delimiter)
{
    std::vector<std::string> substrings;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while (end != std::string::npos)
    {
        std::string substring = str.substr(start, end - start);
        substrings.push_back(substring);

        start = end + 1;
        end = str.find(delimiter, start);
    }

    // 添加最后一个子串
    std::string substring = str.substr(start);
    substrings.push_back(substring);

    return substrings;
}

void myCompiler::actiongrammar_triggered(void)
{
    int len = sizeof(PATTERNS) / 8;

    QString tabName = "文法";
    ShowWidgetTab *tab = new ShowWidgetTab(this);
    int tabIndex = this->ui.tabWidget_show->addTab(tab, tabName);

    // 制作表头
    int maxColumns = 0;
    char delimiter = ',';
    for (int index = 0; index < len; ++index)
    {
        string str = PATTERNS[index];
        int count = std::count(str.begin(), str.end(), delimiter);
        if (count > maxColumns)
            maxColumns = count;
    }
    ++maxColumns; // 考虑到实际列数要比逗号数多1
    tab->textTable->setColumnCount(maxColumns);

    QStringList headers;
    headers << "L"; // 第一列标题
    for (int index = 1; index < maxColumns; ++index)
        headers << QString::fromStdString("R" + std::to_string(index));
    tab->textTable->setHorizontalHeaderLabels(headers);
    tab->textTable->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #D1F1FF ; color: #4463B9; font-weight: bold;}");

    // 填上数据
    tab->textTable->setRowCount(len);
    for (int rowIndex = 0; rowIndex < len; ++rowIndex)
    {
        string str = PATTERNS[rowIndex];
        vector<string> split = splitString(str, delimiter); // 假设split_string已定义
        for (int colIndex = 0; colIndex < split.size(); ++colIndex)
        {
            QTableWidgetItem *newItem = new QTableWidgetItem(QString::fromStdString(split[colIndex]));
            tab->textTable->setItem(rowIndex, colIndex, newItem);
        }
    }

    tab->textTable->resizeColumnsToContents();
    tab->textTable->resizeRowsToContents();
    tab->textTable->verticalHeader()->hide();
    tab->textTable->show();

    Message = ">>> 文法显示完毕！";
    this->message_show();
    this->ui.tabWidget_show->setCurrentIndex(tabIndex);
}

//@func : 词法分析
void myCompiler::actionLexicalAnalyze_triggered(void)
{
    auto startTime = chrono::high_resolution_clock::now(); // 开始时间

    this->isLex = false;
    this->isSyn = false;
    this->isSem = false;
    this->isItermediate = false;
    this->isObject = false;

    // 删除所有临时文件
    deleteFolderContents(QString::fromStdString(folderPath));

    // 更新analyzer
    this->anaylzer.refresh();

    this->ui.textBrowser_log->clear();
    Message.clear();
    Message = ">>> 欢迎使用类C编译器，您正在进行词法分析模块~\n";
    Message += ">>> 正在读取源代码文件...";
    this->message_show();
    if (!ifFileAvailable())
    {
        Message = ">>> 源代码文件读取失败！";
        this->message_show();
        return;
    }
    Message = ">>> 源代码文件读取完毕！";
    this->message_show();

    // 刷新
    this->ui.tabWidget_show->clear();

    Message = ">>> 正在进行词法分析，请稍后...\n";
    Message += ">>> 1. 判断能否生成基于LR1的ActionGOTO表...";
    this->message_show();
    if (!this->isLR1)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法生成基于LR1的ActionGOTO表，词法分析中断！"));
        Message = ">>> 【错误】无法生成基于LR1的ActionGOTO表，词法分析中断！";
        this->message_show();
        return;
    }
    Message = ">>> 2. 进入词法分析...";
    this->message_show();
    if (this->anaylzer.lexAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】词法分析错误！"));
        Message = ">>> 【错误】词法分析错误！";
        this->message_show();
        return;
    }
    else
        this->isLex = true;
    Message = ">>> 源代码的词法分析已完成！";
    this->message_show();

    auto endTime = chrono::high_resolution_clock::now();                                   // 结束时间
    auto duration = chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime); // 计算持续时间
    Message = ">>> 词法分析运行时间：" + to_string(duration.count()) + "毫秒。\n";
    this->message_show();
}

void myCompiler::lexAnalyzerTokenTypeShow(void)
{
    ifstream in((folderPath + "lexOutputToken.txt").c_str());
    ifstream in_2((folderPath + "lexOutputType.txt").c_str());
    if (!in.is_open())
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法得到词法分析Token数据！"));
        Message = ">>> 【错误】无法得到词法分析Token数据！";
        this->message_show();
        return;
    }
    if (!in_2.is_open())
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法得到词法分析Type数据！"));
        Message = ">>> 【错误】无法得到词法分析Type数据！";
        this->message_show();
        return;
    }

    QString tabName = "词法分析Token-Type对应表";
    ShowWidgetTab *tab = new ShowWidgetTab(this);
    int tabIndex = this->ui.tabWidget_show->addTab(tab, tabName);

    int row = count_lines(folderPath + "lexOutputToken.txt");
    int col = 2;
    tab->textTable->setColumnCount(col);
    tab->textTable->setRowCount(row);
    tab->textTable->setWindowTitle("词法分析Token-Type对应表");

    QStringList m_Header;
    m_Header.append("Token");
    m_Header.append("Type");
    tab->textTable->setHorizontalHeaderLabels(m_Header);
    tab->textTable->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #D1F1FF ; color: #4463B9; font-weight: bold;}");
    tab->textTable->verticalHeader()->setStyleSheet("QHeaderView::section { background-color: #F9FDFF; }");
    tab->textTable->setColumnWidth(0, 200);
    tab->textTable->setColumnWidth(1, 200);

    string line_token, line_type;
    for (int i = 0; i < row; i++)
    {
        if (getline(in, line_token) && getline(in_2, line_type))
        {
            tab->textTable->setRowHeight(i, 30);

            tab->textTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(line_token)));
            tab->textTable->item(i, 0)->setTextAlignment(Qt::AlignCenter);

            tab->textTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(line_type)));
            tab->textTable->item(i, 1)->setTextAlignment(Qt::AlignCenter);
        }
    }

    in.close();
    in_2.close();

    tab->textTable->show();

    Message = ">>> 词法分析Token-Type对应表显示完毕！";
    this->message_show();
    this->ui.tabWidget_show->setCurrentIndex(tabIndex);
}

void myCompiler::actionToken_Type_triggered(void)
{
    if (this->isLex)
        this->lexAnalyzerTokenTypeShow();
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("【警告】请先进行词法分析！"));
        Message = ">>> 【警告】请先进行词法分析！";
        this->message_show();
    }
}

void myCompiler::actionLexAnalyzer_triggered(void)
{
    // 加载 SVG 文件
    QString svgFilePath = "../C-basedComplier/src/lexAnalyzer.svg";
    QFile svgFile(svgFilePath);
    qDebug() << svgFilePath;
    if (!svgFile.exists())
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法打开SVG文件！"));
        Message = ">>> 【错误】无法打开SVG文件！";
        this->message_show();
        return;
    }

    // 创建用于展示 SVG 的 QGraphicsView 和 QGraphicsScene
    QGraphicsScene *scene = new QGraphicsScene(this);
    ZoomableGraphicsView *view = new ZoomableGraphicsView(scene, this);
    QSvgRenderer *renderer = new QSvgRenderer(svgFilePath, this);

    // 将 SVG 文件渲染到场景中
    QGraphicsSvgItem *svgItem = new QGraphicsSvgItem();
    svgItem->setSharedRenderer(renderer);
    scene->addItem(svgItem);
    svgItem->setScale(2.5);

    // 设置 QGraphicsView 的一些属性以优化展示
    view->setScene(scene);
    view->setAlignment(Qt::AlignCenter);
    view->setRenderHint(QPainter::Antialiasing);
    view->setWindowTitle(tr("词法分析状态图"));

    // 将 QGraphicsView 作为新标签添加到 tabWidget_show 中
    QString tabName = tr("词法分析状态图");
    int tabIndex = this->ui.tabWidget_show->addTab(view, tabName);
    this->ui.tabWidget_show->setCurrentIndex(tabIndex);

    Message = ">>> 词法分析状态图显示完毕！";
    this->message_show();
}

//@func : 语法分析
void myCompiler::actionSyntacticAnalyze_triggered(void)
{
    auto startTime = chrono::high_resolution_clock::now(); // 开始时间

    this->isLex = false;
    this->isSyn = false;
    this->isSem = false;
    this->isItermediate = false;
    this->isObject = false;

    // 删除所有临时文件
    deleteFolderContents(QString::fromStdString(folderPath));

    // 更新analyzer
    this->anaylzer.refresh();

    this->ui.textBrowser_log->clear();
    Message.clear();
    Message = ">>> 欢迎使用类C编译器，您正在进行语法分析模块~\n";
    Message += ">>> 正在读取源代码文件...";
    this->message_show();
    if (!ifFileAvailable())
    {
        Message = ">>> 源代码文件读取失败！";
        this->message_show();
        return;
    }
    Message = ">>> 源代码文件读取完毕！";
    this->message_show();

    // 刷新
    this->ui.tabWidget_show->clear();

    Message = ">>> 正在进行语法分析，请稍后...\n";
    Message += ">>> 1. 判断能否生成基于LR1的ActionGOTO表...";
    this->message_show();
    if (!this->isLR1)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法生成基于LR1的ActionGOTO表，词法分析中断！"));
        Message = ">>> 【错误】无法生成基于LR1的ActionGOTO表，词法分析中断！";
        this->message_show();
        return;
    }
    Message = ">>> 2. 进入词法分析...";
    this->message_show();
    if (this->anaylzer.lexAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】词法分析错误！"));
        Message += ">>> 【错误】词法分析错误！";
        this->message_show();
        return;
    }
    else
        this->isLex = true;
    Message = ">>> 3. 进入语法分析...";
    if (this->anaylzer.synAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】语法分析错误！"));
        Message += ">>> 【错误】语法分析错误！";
        this->message_show();
        return;
    }
    else
        this->isSyn = true;
    Message += ">>> 源代码的语法分析已完成！";
    this->message_show();

    auto endTime = chrono::high_resolution_clock::now();                                   // 结束时间
    auto duration = chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime); // 计算持续时间
    Message = ">>> 语法分析运行时间：" + to_string(duration.count()) + "毫秒。\n";
    this->message_show();
}

void myCompiler::actionReductionProcess_triggered(void)
{
    if (this->isSyn)
        this->SyntacticAnalyze_show();
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("【警告】请先进行语法分析！"));
        Message = ">>> 【警告】请先进行语法分析！\n";
        this->message_show();
    }
}

void myCompiler::SyntacticAnalyze_show(void)
{

    ifstream in((folderPath + synProcess).c_str());
    if (!in.is_open())
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法得到语法分析归约过程！"));
        Message = ">>> 【错误】无法得到语法分析归约过程！\n";
        this->message_show();
        return;
    }

    QString tabName = "语法分析归约过程";
    ShowWidgetTab *tab = new ShowWidgetTab(this);
    int tabIndex = this->ui.tabWidget_show->addTab(tab, tabName);

    int row = count_lines(folderPath + synProcess);
    int col = 3;
    tab->textTable->setColumnCount(col);
    tab->textTable->setRowCount(row / col + 1);
    tab->textTable->setWindowTitle("语法分析归约过程");

    QStringList m_Header;

    m_Header.append("Status Stack");
    m_Header.append("Symbol Stack");
    m_Header.append("Movement");
    tab->textTable->setHorizontalHeaderLabels(m_Header);
    tab->textTable->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #D1F1FF ; color: #4463B9; font-weight: bold;}");
    tab->textTable->verticalHeader()->setStyleSheet("QHeaderView::section { background-color: #F9FDFF; }");
    tab->textTable->setColumnWidth(0, 200);
    tab->textTable->setColumnWidth(1, 200);
    tab->textTable->setColumnWidth(2, 400);

    string content;
    for (int i = 0; i < row; i++)
    {
        tab->textTable->setRowHeight(i, 30);
        getline(in, content);
        QTableWidgetItem *item = new QTableWidgetItem(QString::number(i / col));
        tab->textTable->setVerticalHeaderItem(i / col, item);
        tab->textTable->setItem(i / col, i % col, new QTableWidgetItem(QString::fromStdString(content)));
        tab->textTable->item(i / col, i % col)->setTextAlignment(Qt::AlignCenter);
    }
    tab->textTable->show();
    Message = ">>> 语法分析归约过程显示完毕！";
    this->message_show();
    this->ui.tabWidget_show->setCurrentIndex(tabIndex);
}

void myCompiler::actionitemSet_triggered(void)
{
    // 加载文本文件
    QString textFilePath = (srcPath + "itemSet.txt").c_str(); // 修改文件路径和扩展名
    QFile textFile(textFilePath);
    qDebug() << textFilePath;
    if (!textFile.exists())
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法打开语法分析项目集展示文件！"));
        Message = ">>> 【错误】无法打开语法分析项目集展示文件！\n";
        this->message_show();
        return;
    }

    if (!textFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法打开语法分析项目集展示文件！"));
        Message = ">>> 【错误】无法打开语法分析项目集展示文件！\n";
        this->message_show();
        return;
    }

    QTextStream in(&textFile);
    QString fileContent = in.readAll();
    textFile.close();

    // 创建用于展示文本的 QTextEdit
    QTextEdit *textEdit = new QTextEdit(this);
    textEdit->setPlainText(fileContent); // 将文件内容设置到 QTextEdit

    // 设置 QTextEdit 的一些属性以优化展示
    textEdit->setReadOnly(true); // 设为只读，因为我们只展示文本
    textEdit->setWindowTitle(tr("语法分析项目集划分"));

    // 将 QTextEdit 作为新标签添加到 tabWidget_show 中
    QString tabName = tr("语法分析项目集划分");
    int tabIndex = this->ui.tabWidget_show->addTab(textEdit, tabName);
    this->ui.tabWidget_show->setCurrentIndex(tabIndex);

    Message = ">>> 语法分析项目集划分展示完毕！\n";
    this->message_show();
}

void myCompiler::actionTree_triggered(void)
{
    if (this->isSyn)
    {
        string order;
        // 注意：确保synTreeImage变量最后没有包含".svg"，因为这里已经添加
        order = string("dot -Tsvg ") + folderPath + synTree + string(" -o ") + folderPath + synTreeImage + ".svg";
        system(order.c_str());

        // 创建 QGraphicsView 和 QGraphicsScene
        ZoomableGraphicsView *view = new ZoomableGraphicsView(this);
        QGraphicsScene *scene = new QGraphicsScene(view);
        view->setScene(scene);

        // 加载并显示语法树的 SVG 图像
        QSvgRenderer *renderer = new QSvgRenderer(QString::fromStdString(folderPath + synTreeImage + ".svg"), this);
        QGraphicsSvgItem *svgItem = new QGraphicsSvgItem();
        svgItem->setSharedRenderer(renderer);
        scene->addItem(svgItem);

        // 根据需要调整 SVG 图像的初始缩放
        svgItem->setScale(1.0); // 或者任何其他合适的缩放因子

        // 将 QGraphicsView 作为新标签添加到 tabWidget_show 中
        QString tabName = tr("语法分析树");
        int tabIndex = this->ui.tabWidget_show->addTab(view, tabName);
        this->ui.tabWidget_show->setCurrentIndex(tabIndex);
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"), tr("【警告】请先完成语法分析过程！"));
        Message = ">>> 【警告】请先完成语法分析过程！";
        this->message_show();
    }
}

void myCompiler::actionActionGOTO_triggered()
{

    if (!this->isSyn)
    {
        QMessageBox::warning(this, tr("Warning"), tr("【警告】请先完成语法分析过程！"));
        Message = ">>> 【警告】请先完成语法分析过程！";
        this->message_show();
        return;
    }

    QString csvFilePath = (srcPath + "ActionGO.csv").c_str();
    QFile file(csvFilePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Error", "无法打开ActionGO.csv文件！" + csvFilePath);
        Message = ">>> 【错误】无法打开ActionGO.csv文件！";
        this->message_show();
        return;
    }

    QTextStream in(&file);
    QVector<QStringList> rowsData;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList rowValues = line.split(",");
        for (QString &value : rowValues)
            value = value.trimmed().replace("\"", ""); // 去除引号和首尾空格
        rowsData.append(rowValues);
    }
    file.close();

    if (rowsData.isEmpty())
    {
        QMessageBox::warning(this, tr("Warning"), tr("【警告】CSV文件为空或格式不正确！"));
        return;
    }

    ShowWidgetTab *tab = new ShowWidgetTab(this);
    QString tabName = "ActionGOTO表";
    int tabIndex = this->ui.tabWidget_show->addTab(tab, tabName);

    // 配置表格
    QStringList headerLabels = rowsData.first(); // 使用第一行作为表头
    tab->textTable->setColumnCount(headerLabels.count());
    tab->textTable->setHorizontalHeaderLabels(headerLabels);
    tab->textTable->setRowCount(rowsData.count() - 1); // 减去表头行

    tab->textTable->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #D1F1FF ; color: #4463B9; font-weight: bold;}");

    // 填充表格数据并设置第一列背景色为淡红色
    for (int i = 1; i < rowsData.count(); ++i)
    {
        for (int j = 0; j < rowsData[i].count(); ++j)
        {
            QTableWidgetItem *item = new QTableWidgetItem(rowsData[i][j]);
            if (j == 0)
            { // 第一列
                item->setBackground(QBrush(QColor("#F9FDFF")));
            }
            tab->textTable->setItem(i - 1, j, item);
        }
    }
    tab->textTable->verticalHeader()->setVisible(false);
    tab->textTable->resizeColumnsToContents(); // 可选，调整列宽以适应内容

    Message = ">>> ActionGO表展示完毕！";
    this->message_show();
    this->ui.tabWidget_show->setCurrentIndex(tabIndex);
}

//@func : 语义分析
void myCompiler::actionSemanticAnalyze_triggered(void)
{
    auto startTime = chrono::high_resolution_clock::now(); // 开始时间

    this->isLex = false;
    this->isSyn = false;
    this->isSem = false;
    this->isItermediate = false;
    this->isObject = false;

    // 删除所有临时文件
    deleteFolderContents(QString::fromStdString(folderPath));

    // 更新analyzer
    this->anaylzer.refresh();

    this->ui.textBrowser_log->clear();
    Message.clear();
    Message = ">>> 欢迎使用类C编译器，您正在进行语义分析模块~\n";
    Message += ">>> 正在读取源代码文件...";
    this->message_show();
    if (!ifFileAvailable())
    {
        Message = ">>> 源代码文件读取失败！";
        this->message_show();
        return;
    }
    Message = ">>> 源代码文件读取完毕！";
    this->message_show();

    Message = ">>> 正在进行语义分析，请稍后...\n";
    Message += ">>> 1. 判断能否生成基于LR1的ActionGOTO表...";
    this->message_show();
    if (!this->isLR1)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法生成基于LR1的ActionGOTO表，词法分析中断！"));
        Message = ">>> 【错误】无法生成基于LR1的ActionGOTO表，词法分析中断！";
        this->message_show();
        return;
    }
    Message = ">>> 2. 进入词法分析...";
    this->message_show();
    if (this->anaylzer.lexAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】词法分析错误！"));
        Message = ">>> 【错误】词法分析错误！";
        this->message_show();
        return;
    }
    else
        this->isLex = true;
    Message = ">>> 3. 进入语法分析...";
    if (this->anaylzer.synAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】语法分析错误！"));
        Message += ">>> 【错误】语法分析错误！";
        this->message_show();
        return;
    }
    else
        this->isSyn = true;
    this->message_show();
    Message = ">>> 4. 进入语义分析...";
    this->message_show();
    if (this->anaylzer.semAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】语义分析错误！"));
        Message = ">>> 【错误】语义分析错误！";
        this->message_show();
        return;
    }
    else
        this->isSem = true;
    Message = ">>> 源代码的语义分析已完成！";
    this->message_show();

    auto endTime = chrono::high_resolution_clock::now();                                   // 结束时间
    auto duration = chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime); // 计算持续时间
    Message = ">>> 语义分析运行时间：" + to_string(duration.count()) + "毫秒。\n";
    this->message_show();
}

//@func : 中间代码生成
void myCompiler::actionIntermediateCode_triggered(void)
{
    auto startTime = chrono::high_resolution_clock::now(); // 开始时间

    this->isLex = false;
    this->isSyn = false;
    this->isSem = false;
    this->isItermediate = false;
    this->isObject = false;

    // 删除所有临时文件
    deleteFolderContents(QString::fromStdString(folderPath));

    // 更新analyzer
    this->anaylzer.refresh();

    this->ui.textBrowser_log->clear();
    Message.clear();
    Message = ">>> 欢迎使用类C编译器，您正在进行中间代码生成模块~\n";
    Message += ">>> 正在读取源代码文件...";
    this->message_show();
    if (!ifFileAvailable())
    {
        Message = ">>> 源代码文件读取失败！";
        this->message_show();
        return;
    }
    Message = ">>> 源代码文件读取完毕！";
    this->message_show();

    Message = ">>> 正在进行语义分析，请稍后...\n";
    Message += ">>> 1. 判断能否生成基于LR1的ActionGOTO表...";
    this->message_show();
    if (!this->isLR1)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法生成基于LR1的ActionGOTO表，词法分析中断！"));
        Message = ">>> 【错误】无法生成基于LR1的ActionGOTO表，词法分析中断！";
        this->message_show();
        return;
    }
    Message = ">>> 2. 进入词法分析...";
    this->message_show();
    if (this->anaylzer.lexAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】词法分析错误！"));
        Message = ">>> 【错误】词法分析错误！";
        this->message_show();
        return;
    }
    else
        this->isLex = true;
    Message = ">>> 3. 进入语法分析...";
    if (this->anaylzer.synAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】语法分析错误！"));
        Message += ">>> 【错误】语法分析错误！";
        this->message_show();
        return;
    }
    else
        this->isSyn = true;
    this->message_show();
    Message = ">>> 4. 进入语义分析...";
    this->message_show();
    if (this->anaylzer.semAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】语义分析错误！"));
        Message = ">>> 【错误】语义分析错误！";
        this->message_show();
        return;
    }
    else
        this->isSem = true;
    Message = ">>> 5. 进入中间代码生成...";
    this->message_show();
    if (this->anaylzer.interAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】中间代码生成错误！"));
        Message = ">>> 【错误】中间代码生成错误！";
        this->message_show();
        return;
    }
    else
        this->isItermediate = true;
    Message = ">>> 源代码的中间代码生成已完成！";
    this->message_show();

    auto endTime = chrono::high_resolution_clock::now();                                   // 结束时间
    auto duration = chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime); // 计算持续时间
    Message = ">>> 中间代码生成运行时间：" + to_string(duration.count()) + "毫秒。\n";
    this->message_show();
}

void myCompiler::actioninterCode_triggered()
{
    if (!this->isItermediate)
    {
        QMessageBox::warning(this, tr("Warning"), tr("【警告】请先完成语义分析过程！"));
        Message = ">>> 【警告】请先完成语义分析过程！";
        this->message_show();
        return;
    }
    ifstream in((folderPath + semCode).c_str());
    if (!in.is_open())
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法打开四元式文件！"));
        Message = ">>> 无法打开四元式文件！";
        this->message_show();
        return;
    }

    QString tabName = "四元式";
    ShowWidgetTab *tab = new ShowWidgetTab(this);
    int tabIndex = this->ui.tabWidget_show->addTab(tab, tabName);

    int row = count_lines(folderPath + semCode);
    int col = 4;
    tab->textTable->setColumnCount(col);
    tab->textTable->setRowCount(row);
    tab->textTable->setWindowTitle("四元式");

    QStringList m_Header;

    m_Header.append("操作符");
    m_Header.append("左操作数");
    m_Header.append("右操作数");
    m_Header.append("结果");
    tab->textTable->setHorizontalHeaderLabels(m_Header);
    tab->textTable->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #D1F1FF ; color: #4463B9; font-weight: bold;}");
    tab->textTable->verticalHeader()->setStyleSheet("QHeaderView::section { background-color: #F9FDFF; }");
    tab->textTable->setColumnWidth(0, 100);
    tab->textTable->setColumnWidth(1, 100);
    tab->textTable->setColumnWidth(2, 100);
    tab->textTable->setColumnWidth(3, 100);

    string Operator, Operand1, Operand2, Result, mid;
    for (int i = 0; i < row; i++)
    {

        tab->textTable->setRowHeight(i, 30);

        in >> mid >> Operator >> mid >> Operand1 >> mid >> Operand2 >> mid >> Result >> mid;

        // 列标
        QTableWidgetItem *item = new QTableWidgetItem(QString::number(i));
        tab->textTable->setVerticalHeaderItem(i, item);
        //
        tab->textTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(Operator)));
        tab->textTable->item(i, 0)->setTextAlignment(Qt::AlignCenter);
        tab->textTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(Operand1)));
        tab->textTable->item(i, 1)->setTextAlignment(Qt::AlignCenter);
        tab->textTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(Operand2)));
        tab->textTable->item(i, 2)->setTextAlignment(Qt::AlignCenter);
        tab->textTable->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(Result)));
        tab->textTable->item(i, 3)->setTextAlignment(Qt::AlignCenter);
    }
    tab->textTable->show();

    Message = ">>> 四元式展示完毕！";
    this->message_show();
    this->ui.tabWidget_show->setCurrentIndex(tabIndex);
}

void myCompiler::actionfuncBlock_triggered()
{
    if (!this->isItermediate)
    {
        QMessageBox::warning(this, tr("Warning"), tr("【警告】请先完成中间代码生成过程！"));
        Message = ">>> 【警告】请先完成中间代码生成过程！";
        this->message_show();
        return;
    }

    QString svgFilePath = (srcPath + "AllFuncBlocks.svg").c_str();
    QFile svgFile(svgFilePath);
    qDebug() << svgFilePath;
    if (!svgFile.exists())
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法打开基本块-流图的SVG文件！"));
        Message = ">>> 【错误】无法打开基本块-流图的SVG文件！";
        this->message_show();
        return;
    }

    // 创建用于展示 SVG 的 QGraphicsView 和 QGraphicsScene
    QGraphicsScene *scene = new QGraphicsScene(this);
    ZoomableGraphicsView *view = new ZoomableGraphicsView(scene, this);
    QSvgRenderer *renderer = new QSvgRenderer(svgFilePath, this);

    // 将 SVG 文件渲染到场景中
    QGraphicsSvgItem *svgItem = new QGraphicsSvgItem();
    svgItem->setSharedRenderer(renderer);
    scene->addItem(svgItem);
    svgItem->setScale(2.5);

    // 设置 QGraphicsView 的一些属性以优化展示
    view->setScene(scene);
    view->setAlignment(Qt::AlignCenter);
    view->setRenderHint(QPainter::Antialiasing);
    view->setWindowTitle(tr("基本块-流图展示图"));

    // 将 QGraphicsView 作为新标签添加到 tabWidget_show 中
    QString tabName = tr("基本块-流图展示图");
    int tabIndex = this->ui.tabWidget_show->addTab(view, tabName);
    this->ui.tabWidget_show->setCurrentIndex(tabIndex);

    Message = ">>> 基本块-流图展示图显示完毕！";
    this->message_show();
}

void myCompiler::actionwaitUseTable_triggered()
{
    if (!this->isItermediate)
    {
        QMessageBox::warning(this, tr("Warning"), tr("【警告】请先完成中间代码生成过程！"));
        Message = ">>> 【警告】请先完成中间代码生成过程！";
        this->message_show();
        return;
    }

    // 展示待用-活跃信息表
    ifstream in((folderPath + intCode).c_str());

    if (!in.is_open())
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法打开中间代码生成文件！"));
        return;
    }

    QString tabName = "待用-活跃信息表";
    ShowWidgetTab *tab = new ShowWidgetTab(this);
    int tabIndex = this->ui.tabWidget_show->addTab(tab, tabName);

    int row = count_lines(folderPath + intCode);
    int col = 5;
    tab->textTable->setColumnCount(col);
    tab->textTable->setRowCount(row);
    tab->textTable->setWindowTitle("待用-活跃信息表");

    QStringList m_Header;

    m_Header.append("标签");
    m_Header.append("四元式");
    m_Header.append("左值");
    m_Header.append("左操作数");
    m_Header.append("右操作数");
    tab->textTable->setHorizontalHeaderLabels(m_Header);
    tab->textTable->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #D1F1FF ; color: #4463B9; font-weight: bold;}");
    tab->textTable->verticalHeader()->setStyleSheet("QHeaderView::section { background-color: #F9FDFF; }");

    tab->textTable->setColumnWidth(0, 80);
    tab->textTable->setColumnWidth(1, 200);
    tab->textTable->setColumnWidth(2, 90);
    tab->textTable->setColumnWidth(3, 90);
    tab->textTable->setColumnWidth(4, 90);

    string content;
    int colCount = 0;
    for (int i = 0; i < row; i++)
    {

        tab->textTable->setRowHeight(i, 30);

        in >> content;

        if (content.find('[') != std::string::npos && content.find('[') != std::string::npos)
        {
            QTableWidgetItem *item = new QTableWidgetItem("Function");
            tab->textTable->setVerticalHeaderItem(i, item);
            tab->textTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(content)));
            tab->textTable->item(i, 0)->setTextAlignment(Qt::AlignCenter);
            colCount = 0;
            continue;
        }
        else if (content.find(':') != std::string::npos)
        {
            QTableWidgetItem *item = new QTableWidgetItem(" ");
            tab->textTable->setVerticalHeaderItem(i, item);
            tab->textTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(content)));
            tab->textTable->item(i, 0)->setTextAlignment(Qt::AlignCenter);
            colCount = 0;
            continue;
        }
        else
        {
            colCount++;
            QTableWidgetItem *item = new QTableWidgetItem(" ");
            tab->textTable->setVerticalHeaderItem(i, item);
            tab->textTable->setItem(i, colCount, new QTableWidgetItem(QString::fromStdString(content)));
            tab->textTable->item(i, colCount)->setTextAlignment(Qt::AlignCenter);
            if (colCount >= 4)
                colCount = 0;
            else
                --i;
        }
    }
    tab->textTable->show();

    Message = ">>> 待用-活跃信息表展示完毕！";
    this->message_show();
    this->ui.tabWidget_show->setCurrentIndex(tabIndex);
}

//@func : 目标代码生成
void myCompiler::actionObjectCode_triggered(void)
{
    auto startTime = chrono::high_resolution_clock::now(); // 开始时间

    this->isLex = false;
    this->isSyn = false;
    this->isSem = false;
    this->isItermediate = false;
    this->isObject = false;

    // 删除所有临时文件
    deleteFolderContents(QString::fromStdString(folderPath));

    // 更新analyzer
    this->anaylzer.refresh();

    this->ui.textBrowser_log->clear();
    Message.clear();
    Message = ">>> 欢迎使用类C编译器，您正在进行目标代码生成模块~\n";
    Message += ">>> 正在读取源代码文件...";
    this->message_show();
    if (!ifFileAvailable())
    {
        Message = ">>> 源代码文件读取失败！";
        this->message_show();
        return;
    }
    Message = ">>> 源代码文件读取完毕！";
    this->message_show();

    Message = ">>> 正在进行语义分析，请稍后...\n";
    Message += ">>> 1. 判断能否生成基于LR1的ActionGOTO表...";
    this->message_show();
    if (!this->isLR1)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法生成基于LR1的ActionGOTO表，词法分析中断！"));
        Message = ">>> 【错误】无法生成基于LR1的ActionGOTO表，词法分析中断！";
        this->message_show();
        return;
    }
    Message = ">>> 2. 进入词法分析...";
    this->message_show();
    if (this->anaylzer.lexAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】词法分析错误！"));
        Message = ">>> 【错误】词法分析错误！";
        this->message_show();
        return;
    }
    else
        this->isLex = true;
    Message = ">>> 3. 进入语法分析...";
    if (this->anaylzer.synAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】语法分析错误！"));
        Message += ">>> 【错误】语法分析错误！";
        this->message_show();
        return;
    }
    else
        this->isSyn = true;
    this->message_show();
    Message = ">>> 4. 进入语义分析...";
    this->message_show();
    if (this->anaylzer.semAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】语义分析错误！"));
        Message = ">>> 【错误】语义分析错误！";
        this->message_show();
        return;
    }
    else
        this->isSem = true;
    Message = ">>> 5. 进入中间代码生成...";
    this->message_show();
    if (this->anaylzer.interAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】中间代码生成错误！"));
        Message = ">>> 【错误】中间代码生成错误！";
        this->message_show();
        return;
    }
    else
        this->isItermediate = true;
    Message = ">>> 6. 进入目标代码生成...";
    if (this->anaylzer.objAnalyzer() == RETURN_ERROR)
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】目标代码生成错误！"));
        Message = ">>> 【错误】目标代码生成错误！";
        this->message_show();
        return;
    }
    else

        this->isObject = true;
    Message = ">>> 源代码的目标代码生成已完成！";
    this->message_show();

    auto endTime = chrono::high_resolution_clock::now();                                   // 结束时间
    auto duration = chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime); // 计算持续时间
    Message = ">>> 目标代码生成运行时间：" + to_string(duration.count()) + "毫秒。\n";
    this->message_show();
}

void myCompiler::actiongenCode_triggered(void)
{
    if (!this->isObject)
    {
        QMessageBox::warning(this, tr("Warning"), tr("【警告】请先完成目标代码生成过程！"));
        Message = ">>> 【警告】请先完成目标代码生成过程！";
        this->message_show();
        return;
    }

    ifstream in((folderPath + objCode).c_str());
    if (!in.is_open())
    {
        QMessageBox::critical(this, tr("Error"), tr("【错误】无法打开目标代码文件！"));
        Message = ">>> 【错误】无法打开目标代码文件！";
        this->message_show();
        return;
    }

    QString tabName = "目标代码";
    ShowWidgetTab *tab = new ShowWidgetTab(this);
    int tabIndex = this->ui.tabWidget_show->addTab(tab, tabName);

    int row = count_lines(folderPath + objCode);
    int col = 2;
    tab->textTable->setColumnCount(col);
    tab->textTable->setRowCount(row);
    tab->textTable->setWindowTitle("目标代码");

    QStringList m_Header;

    m_Header.append("标签");
    m_Header.append("MIPS汇编指令");
    tab->textTable->setHorizontalHeaderLabels(m_Header);
    tab->textTable->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: #D1F1FF ; color: #4463B9; font-weight: bold;}");
    tab->textTable->verticalHeader()->setStyleSheet("QHeaderView::section { background-color: #F9FDFF; }");
    tab->textTable->setColumnWidth(0, 100);
    tab->textTable->setColumnWidth(1, 300);

    string line;
    for (int i = 0; i < row; i++)
    {

        tab->textTable->setRowHeight(i, 30);
        getline(in, line);

        if (line.find(':') != std::string::npos)
        {
            tab->textTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(line)));
            tab->textTable->item(i, 0)->setTextAlignment(Qt::AlignCenter);
        }
        else
        {
            tab->textTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(line)));
            tab->textTable->item(i, 1)->setTextAlignment(Qt::AlignCenter);
        }
    }
    tab->textTable->show();

    Message = ">>> 目标代码显示完毕！";
    this->message_show();
    this->ui.tabWidget_show->setCurrentIndex(tabIndex);
}

//@func :
void myCompiler::actionSave_ObjectCode_triggered(void)
{
    if (this->isObject)
    {
        string path = folderPath;
        string name = objCode;
        QFile file(QString::fromStdString(path + name));

        QString defaultPath = QString::fromStdString(defaultCodeSavePath + defaultCodeSaveFile);
        QString selectedFilter;
        QString filePath = QFileDialog::getSaveFileName(this, "file saving", defaultPath, "c file (*.c);;all file (*)", &selectedFilter);
        if (!filePath.isEmpty())
        {
            QFile Out(filePath);
            if (file.open(QIODevice::ReadOnly))
            {
                QString content = file.readAll();
                if (Out.open(QIODevice::WriteOnly | QIODevice::Text))
                {
                    QTextStream stream(&Out);
                    stream << content;
                    Out.close();
                }
                file.close();
            }
        }
    }
    else
        QMessageBox::warning(this, tr("Warning"), tr("【警告】还未生成目标代码！"));
}
