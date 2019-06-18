QT       += core gui sql printsupport declarative

QMAKE_CXXFLAGS += -std=gnu++0x

TARGET = optimization-solver
TEMPLATE = app

LIBS += "../optimization-solver-build-desktop/debug/log.dll"
LIBS += "../optimization-solver-build-desktop/debug/vector.dll"
LIBS += "../optimization-solver-build-desktop/debug/compact.dll"
LIBS += "../optimization-solver-build-desktop/debug/set.dll"

SOURCES += main.cpp \
    controller.cpp \
    sqlconnectiondialog.cpp \
    insertsolverdialog.cpp \
    qcustomplot.cpp \
    solverdialog.cpp

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
    qcustomplot.h \
    solverdialog.h

FORMS    += \
    controller.ui \
    sqlconnectiondialog.ui \
    insertsolverdialog.ui \
    solverdialog.ui
