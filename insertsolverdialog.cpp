#include <QFileDialog>

#include "insertsolverdialog.h"

InsertSolverDialog::InsertSolverDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
}

InsertSolverDialog::~InsertSolverDialog()
{
}

QString InsertSolverDialog::methodName() const
{
    return ui.editMethod->text();
}

QString InsertSolverDialog::methodDescription() const
{
    return ui.editDescription->toPlainText();
}

QString InsertSolverDialog::methodPath() const
{
    return ui.editPath->text();
}

void InsertSolverDialog::on_browseButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Select file with solver", "", "DLL (*.dll)");
    ui.editPath->setText(fileName);
}
