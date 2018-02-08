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
    connect(this->m_ui->fileView, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(lineChanged(QListWidgetItem*)));
}

MergeDialog::~MergeDialog(void)
{
    delete m_ui;
}

QVector<std::string> MergeDialog::getBasicVector(const QStringList &list)
{
    QVector<std::string> result;

    for (QString line : list)
        result.push_back(line.toStdString());
    return (result);
}

void MergeDialog::showFileLines(const QVector<MergeDialog::LineCheck> &lines)
{
    for (LineCheck line : lines) {
        QListWidgetItem *item = new QListWidgetItem(line.line);

        item->setCheckState(line.checked ? Qt::Checked : Qt::Unchecked);
        this->m_ui->fileView->addItem(item);
    }
}

void MergeDialog::init(const QString &id) {
    this->m_id = id;
}

void MergeDialog::setProjectPath(const QString &path)
{
    this->m_basePath = path;
}

const QString &MergeDialog::getID(void)
{
    return (this->m_id);
}

void MergeDialog::clean(void)
{
    this->m_files.clear();
    this->m_ui->compareList->clear();
    this->m_ui->fileView->clear();
    this->m_ui->filesList->clear();
}

void MergeDialog::compareChange(int row) {
    if (row < 0)
        return;
    static QStringList textExt = {"cpp", "py", "js", "ts", "gitignore", "txt", "xml", "json",
                           "html", "css", "c", "scss", "h", "hpp", "ui", "pro"};
    QVector<LineCheck> content = this->m_files[this->m_ui->filesList->item(row)->text()];

    this->m_ui->compareList->clear();
    this->m_ui->fileView->clear();
    if (!textExt.contains(QFileInfo(this->m_ui->filesList->item(row)->text()).completeSuffix())) {
        this->m_ui->compareList->addItems({"", "", "", "\t\tEXTENSION CANNOT BE SHOWN"});
        return;
    }
    QFile file(QDir::cleanPath(this->m_basePath + this->m_ui->filesList->item(row)->text()));
    if (file.open(QFile::ReadOnly)) {
        QStringList fileContent = QString(file.readAll()).split("\r\n");
        QStringList mergingContent;

        for (LineCheck line : content)
            mergingContent << line.line;
        dtl::Diff<std::string> d(this->getBasicVector(fileContent).toStdVector(), this->getBasicVector(mergingContent).toStdVector());
        d.compose();
        for (auto line : d.getSes().getSequence())
            if (line.second.type == dtl::SES_ADD) {
                QListWidgetItem *addItem = new QListWidgetItem(QString::fromStdString(line.first));

                addItem->setTextColor(QColor(Qt::green));
                this->m_ui->compareList->addItem(addItem);
            }
            else if (line.second.type == dtl::SES_DELETE) {
                QListWidgetItem *removeItem = new QListWidgetItem(QString::fromStdString(line.first));

                removeItem->setTextColor(QColor(Qt::red));
                this->m_ui->compareList->addItem(removeItem);
            }
    }
    this->showFileLines(content);
}

const QMap<QString, QStringList> MergeDialog::getMergingContent(void) {
    QMap<QString, QStringList> result;

    for (int i = 0; i < this->m_ui->filesList->count(); i++) {
        if (this->m_ui->filesList->item(i)->checkState() == Qt::Unchecked)
            continue;
        QStringList content;

        for (LineCheck line : this->m_files[this->m_ui->filesList->item(i)->text()])
            if (line.checked)
                content << line.line;
        result.insert(this->m_ui->filesList->item(i)->text(), content);
    }
    return (result);
}

void MergeDialog::stateChanged(QListWidgetItem *item)
{
    this->m_selected = this->m_ui->filesList->row(item);
    for (int i = 0; i < this->m_ui->filesList->count(); i++)
        if (this->m_ui->filesList->item(i)->checkState() == Qt::Checked) {
            this->m_ui->replaceButton->setEnabled(true);
            return;
        }
    this->m_ui->replaceButton->setEnabled(false);
}

void MergeDialog::lineChanged(QListWidgetItem *item)
{
    QVector<LineCheck> lines = this->m_files[this->m_ui->filesList->item(this->m_selected - 1)->text()];

    LineCheck line = lines.at(this->m_ui->fileView->row(item));
    qDebug() << line.checked;
    line.checked = Qt::Checked == item->checkState();
    qDebug() << line.checked;
    this->m_files[this->m_ui->filesList->item(this->m_selected - 1)->text()].replace(this->m_ui->fileView->row(item), line);
}

void MergeDialog::prepareFile(const QString &name, const QString &content)
{
    QListWidgetItem *item = new QListWidgetItem(name);
    QVector<LineCheck> lines;

    this->m_selected = 1;
    item->setCheckState(Qt::Checked);
    for (QString line : QString::fromUtf8(QByteArray::fromBase64(content.toUtf8())).split("\r\n"))
        lines.append({line, true});
    this->m_files.insert(name, lines);
    this->m_ui->filesList->addItem(item);
}
