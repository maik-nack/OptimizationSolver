#ifndef SOLVERDIALOG_H
#define SOLVERDIALOG_H

#include <QDialog>

#include "ui_solverdialog.h"
#include "error.h"

class SolverParams : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(unsigned dimArgs READ getDimArgs WRITE setDimArgs NOTIFY dimArgsChanged)
    Q_PROPERTY(unsigned dimParams READ getDimParams WRITE setDimParams NOTIFY dimParamsChanged)

    SolverParams() : dimArgs(0), dimParams(0) {}
    SolverParams(unsigned args, unsigned params) : dimArgs(args), dimParams(params) {}

    unsigned getDimArgs() const { return dimArgs; }
    void setDimArgs(unsigned d) { dimArgs = d; }
    unsigned getDimParams() const { return dimParams; }
    void setDimParams(unsigned d) { dimParams = d; }

signals:
    void dimArgsChanged();
    void dimParamsChanged();

private:
    unsigned dimArgs;
    unsigned dimParams;
};

class SolverDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SolverDialog(const QUrl &source, const SolverParams *params, QWidget *parent = 0);
    ~SolverDialog();

    QString getParameters() const;
    int isSolveByArgs(bool &rc) const;

private:
    Ui::SolverDialogUi ui;
    QObject * root;
    const SolverParams * params;
};

#endif // SOLVERDIALOG_H
