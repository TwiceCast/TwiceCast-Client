#include "IgnoredAdderDialog.hpp"
#include "ui_ignoredadderdialog.h"

IgnoredAdderDialog::IgnoredAdderDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::IgnoredAdderDialog)
{
    m_ui->setupUi(this);
    this->m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

IgnoredAdderDialog::~IgnoredAdderDialog()
{
    delete m_ui;
}

QString IgnoredAdderDialog::getPath() const
{
    return (this->m_ui->matchEditLine->text());
}

void IgnoredAdderDialog::setTreeFile(QTreeWidget *tree)
{
    this->m_tree = tree;
}

QStringList IgnoredAdderDialog::findItems(QTreeWidgetItem *item, int column, const QString &check) const
{
    QStringList list;
    QRegExp regexp("*/" + this->m_tree->topLevelItem(0)->text(0) + "/" + check);

    regexp.setPatternSyntax(QRegExp::Wildcard);
    regexp.setCaseSensitivity(Qt::CaseSensitive);
    if (item != this->m_tree->topLevelItem(0) && regexp.exactMatch(item->text(column)))
        list.append(item->data(2, Qt::DisplayRole).toString().replace(this->m_tree->topLevelItem(0)->text(2) + "/", ""));
    for (int i = 0; i < item->childCount(); i++)
        list.append(this->findItems(item->child(i), column, check));
    return (list);
}

void IgnoredAdderDialog::on_matchEditLine_textChanged(const QString &text)
{
    if (this->m_tree == NULL)
        return;
    this->m_ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(text != "");
    this->m_ui->matchList->clear();
    this->m_ui->matchList->addItems(this->findItems(this->m_tree->topLevelItem(0), 2, text));
}
