QT       += core gui svg svgwidgets


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    LR1Analyzer.cpp \
    analyzer.cpp \
    interCodeGenerator.cpp \
    lexialAnalyzer.cpp \
    main.cpp \
    myCompiler.cpp \
    objCodeGenerator.cpp \
    semanticAnalyzer.cpp \
    syntaxAnalyzer.cpp

HEADERS += \
    analyzer.h \
    interCodeGenerator.h \
    lexialAnalyzer.h \
    myCompiler.h \
    objCodeGenerator.h \
    semanticAnalyzer.h \
    source.h \
    syntaxAnalyzer.h

FORMS += \
    myCompiler.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    myCompiler.qrc

DISTFILES += \
    icon/AI.svg

msvc {
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}
