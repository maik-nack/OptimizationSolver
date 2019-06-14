#ifndef SQLCONNECTIONDIALOG_H
#define SQLCONNECTIONDIALOG_H

#include <QDialog>

#include "ui_sqlconnectiondialog.h"

class SqlConnectionDialog: public QDialog
{
    Q_OBJECT
public:
    SqlConnectionDialog(QWidget *parent = 0);
    ~SqlConnectionDialog();

    QString driverName() const;
    QString databaseName() const;
    QString userName() const;
    QString password() const;
    QString hostName() const;
    int port() const;

private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked()
    { reject(); }

private:
    Ui::SqlConnectionDialogUi ui;
};

#endif // SQLCONNECTIONDIALOG_H
