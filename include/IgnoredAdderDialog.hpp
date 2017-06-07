#ifndef IGNOREDADDERDIALOG_HPP
#define IGNOREDADDERDIALOG_HPP

#include <QTreeWidget>
#include <QDialog>
#include <QPushButton>
#include <QDebug>
#include <QDir>
#include "CustomStyle.hpp"

namespace Ui {
class IgnoredAdderDialog;
}

class IgnoredAdderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IgnoredAdderDialog(QWidget *parent = 0);
    ~IgnoredAdderDialog(void);

public:
    QString getPath(void) const;

public:
    void setTreeFile(QTreeWidget *);

private:
    QStringList findItems(QTreeWidgetItem *item, int column, const QString &check) const;

private slots:
    void on_matchEditLine_textChanged(const QString &arg1);

private:
    Ui::IgnoredAdderDialog *m_ui;
    QTreeWidget *m_tree;
};

#endif // IGNOREDADDERDIALOG_HPP
