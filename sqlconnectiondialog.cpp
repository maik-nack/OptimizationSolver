#include <QSqlDatabase>
#include <QMessageBox>

#include "sqlconnectiondialog.h"

SqlConnectionDialog::SqlConnectionDialog(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);

    QStringList drivers = QSqlDatabase::drivers();

    // remove compat names
    drivers.removeAll("QMYSQL3");
    drivers.removeAll("QOCI8");
    drivers.removeAll("QODBC3");
    drivers.removeAll("QPSQL7");
    drivers.removeAll("QTDS7");

    ui.comboDriver->addItems(drivers);
}

SqlConnectionDialog::~SqlConnectionDialog()
{
}

QString SqlConnectionDialog::driverName() const
{
    return ui.comboDriver->currentText();
}

QString SqlConnectionDialog::databaseName() const
{
    return ui.editDatabase->text();
}

QString SqlConnectionDialog::userName() const
{
    return ui.editUsername->text();
}

QString SqlConnectionDialog::password() const
{
    return ui.editPassword->text();
}

QString SqlConnectionDialog::hostName() const
{
    return ui.editHostname->text();
}

int SqlConnectionDialog::port() const
{
    return ui.portSpinBox->value();
}

void SqlConnectionDialog::on_okButton_clicked()
{
    if (ui.comboDriver->currentText().isEmpty()) {
        QMessageBox::information(this, tr("No database driver selected"),
                                 tr("Please select a database driver"));
        ui.comboDriver->setFocus();
    } else {
        accept();
    }
}
