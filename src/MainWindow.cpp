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
//    connect(this->m_ui->treeFile, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT());
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

    item->setData(2, Qt::UserRole, dir.path());
    for (auto file : this->m_watch.files()) {
        QFileInfo f(file);

        if (f.absoluteDir() == dir.absolutePath()) {
            QTreeWidgetItem *tmp = new QTreeWidgetItem(QStringList(f.fileName()));
            tmp->setData(1, Qt::CheckStateRole, Qt::Checked);
            tmp->setData(2, Qt::UserRole, f.filePath());
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
            if (item->child(i)->data(2, Qt::UserRole).toString() == str)
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
            if (item->child(i)->data(2, Qt::UserRole).toString() == str)
                check = true;
        if (!check)
            result.append(item->child(i));
    }
    return (result);
}

void MainWindow::checkPath(QTreeWidgetItem *item, const QString &path, const QStringList &list)
{
    if (item->data(2, Qt::UserRole).toString() == path) {
        QList<QTreeWidgetItem *> removes = this->getRemovedPaths(item, list);
        QList<QString> adds = this->getAddedPaths(item, list);

        if (adds.size() == 1 && removes.size() == 1) {
            QFileInfo fi(adds.at(0));
            removes.at(0)->setData(0, Qt::DisplayRole, fi.fileName());
            removes.at(0)->setData(2, Qt::UserRole, adds.at(0));
            return;
        }
        for (auto add : adds) {
            QFileInfo fi(add);
            QTreeWidgetItem *newItem = new QTreeWidgetItem(QStringList(fi.fileName()));

            if (fi.isFile())
                newItem->setData(1, Qt::CheckStateRole, Qt::Checked);
            newItem->setData(2, Qt::UserRole, add);
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
    }
}

void MainWindow::projectCreated()
{
    QStringList listNotRemoved;
    QMap<ProjectCreationDialog::ProjectSettings, QString> values = this->m_creation.getValues();

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
    this->m_ui->treeFile->expandAll();
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}
