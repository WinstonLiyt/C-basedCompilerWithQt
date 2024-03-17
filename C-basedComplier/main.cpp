#include "myCompiler.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    myCompiler w;

    w.show();

    return a.exec();
}
