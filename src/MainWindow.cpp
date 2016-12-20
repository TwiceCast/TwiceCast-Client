#include <QDebug>
#include "MainWindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_pathWatched("")
{
    this->m_ui->setupUi(this);
    connect(this->m_ui->actionNewProject, SIGNAL(triggered()), &this->m_creation, SLOT(show()));
    connect(&this->m_creation, SIGNAL(accepted()), this, SLOT(projectCreated()));
    connect(&this->m_watch, SIGNAL(directoryChanged(QString)), this, SLOT(directoryWatchedChanged(QString)));
    connect(&this->m_watch, SIGNAL(fileChanged(QString)), this, SLOT(fileWatchedChanged(QString)));
    connect(this->m_ui->treeFile, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(addPathIgnored(QTreeWidgetItem *, int)));
    connect(this->m_ui->listIgnored->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(checkTreeIgnored()));
    connect(this->m_ui->listIgnored->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(checkTreeIgnored()));
}

MainWindow::~MainWindow(void)
{
    delete (this->m_ui);
}

QStringList MainWindow::getFileList(const QString &path, bool recursively) const
{
    QStringList result;
    QDir dir(path);

    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (auto it : dir.entryInfoList()) {
        result.append(it.filePath());
        if (it.isDir() && it.isReadable() && recursively)
            result.append(this->getFileList(it.filePath(), true));
    }
    return (result);
}

QTreeWidgetItem *MainWindow::getTreeItem(const QString &path) const
{
    QDir dir(path);
    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(dir.dirName()));

    item->setData(2, Qt::DisplayRole, dir.path());
    for (auto file : this->m_watch.files()) {
        QFileInfo f(file);

        if (f.absoluteDir() == dir.absolutePath()) {
            QTreeWidgetItem *tmp = new QTreeWidgetItem(QStringList(f.fileName()));
            tmp->setData(1, Qt::CheckStateRole, Qt::Checked);
            tmp->setData(2, Qt::DisplayRole, f.filePath());
            item->addChild(tmp);
        }
    }
    for (auto dirs : this->m_watch.directories()) {
        if (dirs != path) {
            QDir d(dirs);

            d.cdUp();
            if (d.absolutePath() == dir.absolutePath())
                item->addChild(this->getTreeItem(dirs));
        }
    }
    return (item);
}

QList<QString> MainWindow::getAddedPaths(QTreeWidgetItem *item, const QStringList &list) const
{
    QList<QString> result;

    for (auto str : list) {
        bool check = false;
        for (int i = 0; i < item->childCount() && !check; i++)
            if (item->child(i)->data(2, Qt::DisplayRole).toString() == str)
                check = true;
        if (!check)
            result.append(str);
    }
    return (result);
}

QList<QTreeWidgetItem *> MainWindow::getRemovedPaths(QTreeWidgetItem *item, const QStringList &list) const
{
    QList<QTreeWidgetItem *> result;

    for (int i = 0; i < item->childCount(); i++) {
        bool check = false;
        for (auto str : list)
            if (item->child(i)->data(2, Qt::DisplayRole).toString() == str)
                check = true;
        if (!check)
            result.append(item->child(i));
    }
    return (result);
}

void MainWindow::checkPath(QTreeWidgetItem *item, const QString &path, const QStringList &list)
{
    if (item->data(2, Qt::DisplayRole).toString() == path) {
        QList<QTreeWidgetItem *> removes = this->getRemovedPaths(item, list);
        QList<QString> adds = this->getAddedPaths(item, list);

        if (adds.size() == 1 && removes.size() == 1) {
            QFileInfo fi(adds.at(0));
            removes.at(0)->setData(0, Qt::DisplayRole, fi.fileName());
            removes.at(0)->setData(2, Qt::DisplayRole, adds.at(0));
            return;
        }
        for (auto add : adds) {
            QFileInfo fi(add);
            QTreeWidgetItem *newItem = new QTreeWidgetItem(QStringList(fi.fileName()));

            if (fi.isFile())
                newItem->setData(1, Qt::CheckStateRole, Qt::Checked);
            newItem->setData(2, Qt::DisplayRole, add);
            item->addChild(newItem);
        }
        for (auto remove : removes)
            delete remove;
    }
    else
        for (int i = 0; i < item->childCount(); i++)
            this->checkPath(item->child(i), path, list);
}

void MainWindow::directoryWatchedChanged(const QString &path)
{
    if (!QDir().exists(path))
        this->m_watch.removePath(path);
    else {
        QStringList strList = this->getFileList(path);

        this->m_watch.addPaths(strList);
        this->checkPath(this->m_ui->treeFile->topLevelItem(0), path, strList);
    }
}

void MainWindow::fileWatchedChanged(const QString &path)
{
    QFile file(path);

    if (!file.exists())
        this->m_watch.removePath(path);
    else {
        qDebug() << "File " << path << " has been changed";
        //TODO : FTP Web Socket
    }
}

void MainWindow::addPathIgnored(QTreeWidgetItem *item, int column)
{
    QRegExp regexp;

    if (item == NULL || column != 1)
        return;
    regexp.setPatternSyntax(QRegExp::Wildcard);
    regexp.setCaseSensitivity(Qt::CaseSensitive);
    if (item->data(column, Qt::CheckStateRole) == Qt::Unchecked) {
        QListWidgetItem *newItem = new QListWidgetItem(item->data(2, Qt::DisplayRole).toString());

        newItem->setText(newItem->text().replace(this->m_ui->treeFile->topLevelItem(0)->data(2, Qt::DisplayRole).toString() + "/", ""));
        newItem->setData(Qt::UserRole, item->data(2, Qt::DisplayRole));
        this->m_ui->listIgnored->addItem(newItem);
    }
    else if (item->data(column, Qt::CheckStateRole) == Qt::Checked)
        for (int i = 0; i < this->m_ui->listIgnored->count();) {
            regexp.setPattern("*" + this->m_ui->listIgnored->item(i)->text());
            if (this->m_ui->listIgnored->item(i)->data(Qt::UserRole).isValid() &&
                    this->m_ui->listIgnored->item(i)->data(Qt::UserRole).toString() == item->data(2, Qt::DisplayRole).toString())
                delete this->m_ui->listIgnored->item(i);
            else if (this->m_ui->listIgnored->item(i)->data(Qt::UserRole).isValid() &&
                     this->m_ui->listIgnored->item(i)->data(Qt::UserRole).toString() == "" &&
                     regexp.exactMatch(item->data(2, Qt::DisplayRole).toString())) {
                if (QMessageBox::warning(this, "Remove path matching",
                                         "There is a conflict for reactivating the item with : " +
                                         this->m_ui->listIgnored->item(i)->text() + "\nDelete?",
                                         QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                    delete this->m_ui->listIgnored->item(i);
                else
                    i++;
            }
            else
                i++;
        }
}

QList<QTreeWidgetItem *> MainWindow::findItems(QTreeWidgetItem *item, int column, const QString &check) const
{
    QList<QTreeWidgetItem *> list;
    QRegExp regexp("*" + check);

    regexp.setPatternSyntax(QRegExp::Wildcard);
    regexp.setCaseSensitivity(Qt::CaseSensitive);
    if (regexp.exactMatch(item->text(column)))
        list.append(item);
    for (int i = 0; i < item->childCount(); i++)
        list.append(this->findItems(item->child(i), column, check));
    return (list);
}

void MainWindow::resetFont(QTreeWidgetItem *item)
{
    QFont font(item->font(0));

    font.setStrikeOut(false);
    item->setFont(0, font);
    for (int i = 0; i < item->childCount(); i++)
        this->resetFont(item->child(i));
}

void MainWindow::applyFont(QTreeWidgetItem *item)
{
    QFont font(item->font(0));
    bool allIgnored = true;

    for (int i = 0; i < item->childCount(); i++)
    {
        allIgnored = allIgnored && item->child(i)->font(0).strikeOut();
        if (item->child(i)->childCount() > 0)
            this->applyFont(item->child(i));
    }
    font.setStrikeOut(allIgnored);
    item->setFont(0, font);
}

void MainWindow::checkTreeIgnored(void)
{
    QList<QTreeWidgetItem *> list;
    QListWidgetItem *tableItem;

    this->resetFont(this->m_ui->treeFile->topLevelItem(0));
    for (int i = 0; i < this->m_ui->listIgnored->count(); i++) {
        tableItem = this->m_ui->listIgnored->item(i);
        list = this->findItems(this->m_ui->treeFile->topLevelItem(0), 2,
                               (tableItem->data(Qt::UserRole).isValid() && tableItem->data(Qt::UserRole).toString() != ""
                ? tableItem->data(Qt::UserRole).toString() : tableItem->text()));
        for (auto elem : list) {
            QFont font(elem->font(1));

            font.setStrikeOut(true);
            elem->setFont(0, font);
        }
    }
    this->applyFont(this->m_ui->treeFile->topLevelItem(0));
}

void MainWindow::projectCreated(void)
{
    QMap<ProjectCreationDialog::ProjectSettings, QString> values = this->m_creation.getValues();
    QStringList listNotRemoved;

    this->m_ui->titleLabel->setText(values[ProjectCreationDialog::TITLE]);
    this->m_pathWatched = values[ProjectCreationDialog::PATH];
    if (this->m_pathWatched == "")
        return;
    listNotRemoved = this->m_watch.removePaths(this->m_watch.files());
    listNotRemoved.append(this->m_watch.removePaths(this->m_watch.directories()));
    if (listNotRemoved.size() != 0)
        listNotRemoved = this->m_watch.removePaths(listNotRemoved);
    this->m_ui->watchedDirectory->setText(this->m_pathWatched);
    this->m_ui->treeFile->clear();
    this->m_watch.addPath(this->m_pathWatched);
    this->m_watch.addPaths(this->getFileList(this->m_pathWatched, true));
    this->m_ui->treeFile->addTopLevelItem(this->getTreeItem(this->m_pathWatched));
    this->m_ui->treeFile->setRootIndex(this->m_ui->treeFile->model()->index(0, 0));
    this->m_ui->treeFile->expandAll();
}

void MainWindow::on_actionExit_triggered(void)
{
    this->close();
}
