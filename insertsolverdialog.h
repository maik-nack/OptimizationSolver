#ifndef INSERTSOLVERDIALOG_H
#define INSERTSOLVERDIALOG_H

#include <QDialog>

#include "ui_insertsolverdialog.h"

class InsertSolverDialog: public QDialog
{
    Q_OBJECT
public:
    InsertSolverDialog(QWidget *parent = 0);
    ~InsertSolverDialog();

    QString methodName() const;
    QString methodDescription() const;
    QString methodPath() const;

private slots:
    void on_okButton_clicked()
    { accept(); }
    void on_cancelButton_clicked()
    { reject(); }
    void on_browseButton_clicked();

private:
    Ui::InsertSolverDialogUi ui;
};

#endif // INSERTSOLVERDIALOG_H
