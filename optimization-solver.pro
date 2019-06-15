QT       += core gui

QMAKE_CXXFLAGS += -std=gnu++0x

TARGET = optimization-solver
TEMPLATE = app


SOURCES += main.cpp mainwindow.cpp

HEADERS  += mainwindow.h \
    SHARED_EXPORT.h \
    IVector.h \
    ISolver.h \
    ISet.h \
    IProblem.h \
    ILog.h \
    ICompact.h \
    IBrocker.h \
    error.h

FORMS    += mainwindow.ui
