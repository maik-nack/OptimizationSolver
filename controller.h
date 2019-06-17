#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QWidget>
#include <QDebug>

#include "ui_controller.h"
#include "IBrocker.h"
#include "ISolver.h"
#include "IProblem.h"

QT_FORWARD_DECLARE_CLASS(QSqlError)
QT_FORWARD_DECLARE_CLASS(QSqlRecord)

class Controller : public QWidget, private Ui::Controller
{
    Q_OBJECT

public:
    explicit Controller(QWidget *parent = 0);
    ~Controller();

    QSqlError changeConnection(const QString &driver, const QString &dbName, const QString &host,
                               const QString &user, const QString &passwd, int port = -1);

public slots:
    void changeConnection();
    void currentChanged()
    { updateActions(); }

private slots:
    void on_insertRowAction_triggered();
    void on_deleteRowAction_triggered();
    void on_browseButton_clicked();
    void on_insButton_clicked();
    void on_solveButton_clicked();
    void on_drawButton_clicked();
    void on_saveImageButton_clicked();

signals:
    void statusMessage(const QString &message);

private:
    typedef IBrocker *(* get_brocker_func)();
    IBrocker * _problem_brocker, * _solver_brocker;
    IProblem * _problem;
    ISolver * _solver;

    void updateActions();
    bool select();
    bool checkTable(QSqlRecord record);

};

#endif // CONTROLLER_H
