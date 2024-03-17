#pragma once
#ifndef MYCOMPILER_H
#define MyCOMPILER_H

#include <QtWidgets/QMainWindow>
#include <QKeyEvent>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QString.h>
#include <QFileDialog>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <direct.h>
#include <QFile>
#include <QDir>
#include <io.h>
#include <QTextEdit>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QProcess>
#include <sys/stat.h>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QSvgRenderer>
#include <QGraphicsSvgItem>
#include <QDebug>
#include <qdebug.h>

#include "source.h"
#include "analyzer.h"
#include "ui_myCompiler.h"

using namespace std;

class ZoomableTextEdit : public QTextEdit
{
protected:
    void wheelEvent(QWheelEvent *event) override
    {
        if (event->modifiers() & Qt::ControlModifier)
        {
            const int delta = event->angleDelta().y();
            QFont font = this->font();
            int fontSize = font.pointSize() + (delta > 0 ? 1 : -1);
            fontSize = std::max(1, fontSize); // 防止字体大小小于1
            font.setPointSize(fontSize);
            this->setFont(font);
            event->accept();
        }
        else
        {
            QTextEdit::wheelEvent(event);
        }
    }
};

class CodeWidgetTab : public QWidget
{
    Q_OBJECT //  Q_OBJECT

        public : ZoomableTextEdit *textEdit;
    bool isUnnamed;
    bool isModify;

public:
    CodeWidgetTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        this->textEdit = new ZoomableTextEdit;
        this->isModify = false;
        this->isUnnamed = false;

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(this->textEdit);
        setLayout(layout);
    }
};

class ZoomableGraphicsView : public QGraphicsView
{

public:
    explicit ZoomableGraphicsView(QWidget *parent = nullptr) : QGraphicsView(parent)
    {
    }
    ZoomableGraphicsView(QGraphicsScene *scene, QWidget *parent = nullptr) : QGraphicsView(scene, parent)
    {
    }

protected:
    void wheelEvent(QWheelEvent *event) override
    {
        const qreal scaleFactor = 1.15; // 缩放因子
        if (event->angleDelta().y() > 0)
        {
            // 向上滚动，放大
            scale(scaleFactor, scaleFactor);
        }
        else
        {
            // 向下滚动，缩小
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }
    }
};

class ZoomableTableWidget : public QTableWidget
{
protected:
    void wheelEvent(QWheelEvent *event) override
    {
        if (event->modifiers() & Qt::ControlModifier)
        {
            int delta = event->angleDelta().y();
            QFont font = this->font();
            int fontSize = font.pointSize() + (delta > 0 ? 1 : -1);
            fontSize = std::max(1, fontSize); // Prevent font size less than 1
            font.setPointSize(fontSize);

            this->setFont(font);
            for (int row = 0; row < this->rowCount(); ++row)
            {
                for (int column = 0; column < this->columnCount(); ++column)
                {
                    QTableWidgetItem *item = this->item(row, column);
                    if (item)
                        item->setFont(font);
                }
            }
            event->accept();
        }
        else
        {
            QTableWidget::wheelEvent(event);
        }
    }
};

class ShowWidgetTab : public QWidget
{
    Q_OBJECT //  Q_OBJECT

        public : ZoomableTableWidget *textTable;

public:
    ShowWidgetTab(QWidget *parent = nullptr) : QWidget(parent)
    {
        //  textTable
        this->textTable = new ZoomableTableWidget;

        //
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(this->textTable);
        setLayout(layout);
    }
};

//@func :
class myCompiler : public QMainWindow
{
    Q_OBJECT
private:
    int fileNum;
    bool isLR1;
    bool isLex;
    bool isSyn;
    bool isSem;
    bool isItermediate;
    bool isObject;

private:
    QMap<int, CodeWidgetTab *> tabTextEditMap;
    Compile_Analyzer anaylzer;
    void addCode(const QString &filePath, bool isUnnamed = false);
    void save_textEdit(QString &filePath, QTextEdit *textEdit);
    void lexAnalyzerTokenTypeShow(void);
    void SyntacticAnalyze_show(void);
    bool ifFileAvailable(void);

    void message_show(void);

public:
    myCompiler(QWidget *parent = nullptr);
    ~myCompiler();

private:
    Ui::mycompilerClass ui;

public slots:
    void actionOpen_triggered(void); //
    void actionSave_triggered(void); //
    void actionNew_triggered(void);
    void closeRequest_triggered(int index); //
    void modify_triggered(void);            //
    void actionLexicalAnalyze_triggered(void);
    void actionSyntacticAnalyze_triggered(void);
    void actionSemanticAnalyze_triggered(void);
    void actionIntermediateCode_triggered(void);
    void actionObjectCode_triggered(void);
    void actionTree_triggered(void);
    void actionSave_ObjectCode_triggered(void);
    void actionLexAnalyzer_triggered(void);
    void actionToken_Type_triggered(void);
    void actionReductionProcess_triggered(void);
    void actionitemSet_triggered(void);
    void actionActionGOTO_triggered(void);
    void actioninterCode_triggered(void);
    void actionfuncBlock_triggered(void);
    void actionwaitUseTable_triggered(void);
    void actiongenCode_triggered(void);
    void actiongrammar_triggered(void);
};

#endif // !MYCOMPILER_H
