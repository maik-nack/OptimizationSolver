QT       += core gui sql printsupport

TARGET = optimization-solver
TEMPLATE = app


SOURCES += main.cpp \
    controller.cpp \
    sqlconnectiondialog.cpp \
    insertsolverdialog.cpp \
    qcustomplot.cpp

HEADERS  += \
    SHARED_EXPORT.h \
    IVector.h \
    ISolver.h \
    ISet.h \
    IProblem.h \
    ILog.h \
    ICompact.h \
    IBrocker.h \
    error.h \
    controller.h \
    sqlconnectiondialog.h \
    insertsolverdialog.h \
    qcustomplot.h

FORMS    += \
    controller.ui \
    sqlconnectiondialog.ui \
    insertsolverdialog.ui
