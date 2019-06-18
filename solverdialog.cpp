#include <QDeclarativeView>
#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QVariant>
#include <QList>

#include "solverdialog.h"

SolverDialog::SolverDialog(const QUrl &source, SolverParams * params, QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    this->params = params;
    QDeclarativeView * qmlView = new QDeclarativeView();
    qmlView->rootContext()->setContextProperty("params", params);
    qmlView->setSource(source);
    qmlView->setResizeMode(QDeclarativeView::SizeViewToRootObject);
    ui.vLayout->addWidget(qmlView);
    root = qmlView->rootObject();
}

SolverDialog::~SolverDialog()
{
}

QList<QObject*> getTextInputs(QObject * root)
{
    QList<QObject*> res, children = root->findChildren<QObject*>();
    foreach (QObject * val, children) {
        if (!QString(val->metaObject()->className()).compare("QDeclarativeTextInput"))
            res.append(val);
    }
    return res;
}

QString SolverDialog::getParameters() const
{
    QList<QObject*> children = getTextInputs(root);
    QString res("args:");
    res.append(QString::number(params->getDimArgs()) + " params:" + QString::number(params->getDimParams()) + " ");
    foreach (QObject * val, children) {
        res.append(val->property("objectName").toString() + ":" + val->property("text").toString() + " ");
    }
    res.remove(res.length() - 1, 1);
    return res;
}

int SolverDialog::isSolveByArgs(bool &rc) const
{
    QObject * pararg = root->findChild<QObject*>("pararg");
    if (!pararg) {
        return ERR_ANY_OTHER;
    }
    QString text = pararg->property("text").toString();
    if (text.length() == 0) {
        return ERR_ANY_OTHER;
    }
    rc = QRegExp("[Aa][Rr][Gg][Ss]").exactMatch(text);
    return ERR_OK;
}


