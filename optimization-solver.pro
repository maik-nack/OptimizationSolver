QT       += core gui

TARGET = optimization-solver
TEMPLATE = app


SOURCES += main.cpp mainwindow.cpp

HEADERS  += mainwindow.h \
    SHARED_EXPORT.h \
    IVector.h \
    ISolver.h \
    IProblem.h \
    ILog.h \
    IBrocker.h \
    error.h

FORMS    += mainwindow.ui
