#include "MergeDialog.hpp"
#include "ui_mergedialog.h"

#include <QDebug>

MergeDialog::MergeDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::MergeDialog)
{
    m_ui->setupUi(this);
    connect(this->m_ui->filesList, SIGNAL(currentRowChanged(int)), this, SLOT(compareChange(int)));
    connect(this->m_ui->filesList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(stateChanged(QListWidgetItem*)));
}

MergeDialog::~MergeDialog(void)
{
    delete m_ui;
}

void MergeDialog::init(const QString &id) {
    this->m_id = id;
}

const QString &MergeDialog::getID(void)
{
    return (this->m_id);
}

void MergeDialog::clean(void)
{
    this->m_files.clear();
    this->m_ui->compareList->clear();
    this->m_ui->filesList->clear();
}

void MergeDialog::compareChange(int row) {
    if (row < 0)
        return;
    static QStringList textExt = {"cpp", "py", "js", "ts", "gitignore", "txt", "xml", "json",
                           "html", "css", "c", "scss", "h", "hpp", "ui", "pro"};
    QString content = this->m_files[this->m_ui->filesList->item(row)->text()];

    this->m_ui->compareList->clear();
    if (!textExt.contains(QFileInfo(this->m_ui->filesList->item(row)->text()).completeSuffix())) {
        this->m_ui->compareList->addItems({"", "", "", "\t\tEXTENSION CANNOT BE SHOWN"});
        return;
    }
    this->m_ui->compareList->addItems(content.split("\r\n"));
}

const QMap<QString, QString> &MergeDialog::getMergingContent(void) {
    for (int i = 0; i < this->m_ui->filesList->count(); i++)
        if (this->m_ui->filesList->item(i)->checkState() == Qt::Unchecked)
            this->m_files.remove(this->m_ui->filesList->item(i)->text());
    return (this->m_files);
}

void MergeDialog::stateChanged(QListWidgetItem *item)
{
    qDebug() << item;
}

void MergeDialog::prepareFile(const QString &name, const QString &content)
{
    QListWidgetItem *item = new QListWidgetItem(name);

    item->setCheckState(Qt::Checked);
    qDebug() << name;
    this->m_files.insert(name, QString::fromUtf8(QByteArray::fromBase64(content.toUtf8())));
    this->m_ui->filesList->addItem(item);
}
