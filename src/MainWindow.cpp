#include <QDebug>
#include "MainWindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_pathWatched("")
{
    this->m_ui->setupUi(this);
    connect(&this->m_watch, SIGNAL(directoryChanged(QString)), this, SLOT(directoryWatchedChanged(QString)));
    connect(&this->m_watch, SIGNAL(fileChanged(QString)), this, SLOT(fileWatchedChanged(QString)));
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

QTreeWidgetItem *MainWindow::getTreeItem(const QString &path) const {
    QDir dir(path);
    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList(dir.dirName()));

    for (auto file : this->m_watch.files()) {
        QFileInfo f(file);

        if (f.absoluteDir() == dir.absolutePath())
            item->addChild(new QTreeWidgetItem(QStringList(f.fileName())));
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

void MainWindow::updateTreeView(void) const
{
    this->m_ui->treeFile->clear();
    this->m_ui->treeFile->addTopLevelItem(this->getTreeItem(this->m_pathWatched));
    this->m_ui->treeFile->expandAll();
}

void MainWindow::directoryWatchedChanged(const QString &path)
{
    if (!QDir().exists(path)) {
        qDebug() << "Path " << path << " has been removed";
        this->m_watch.removePath(path);
    }
    else {
        qDebug() << "Path " << path << " has been changed";
        this->m_watch.addPaths(this->getFileList(path));
    }
    this->updateTreeView();
}

void MainWindow::fileWatchedChanged(const QString &path)
{
    QFile file(path);

    if (!file.exists()) {
        qDebug() << "File " << path << " has been removed";
        this->m_watch.removePath(path);
    }
    else {
        qDebug() << "File " << path << " has been changed";
    }
    this->updateTreeView();
}

void MainWindow::on_actionNewProject_triggered()
{
    QStringList listNotRemoved;

    this->m_pathWatched = QFileDialog::getExistingDirectory(this,
                          "Watched Directory",
                          this->m_pathWatched);
    listNotRemoved = this->m_watch.removePaths(this->m_watch.files());
    listNotRemoved.append(this->m_watch.removePaths(this->m_watch.directories()));
    if (listNotRemoved.size() != 0)
        listNotRemoved = this->m_watch.removePaths(listNotRemoved);
    this->m_ui->watchedDirectory->setText((this->m_pathWatched == "" ? "None" : this->m_pathWatched));
    this->m_watch.addPath(this->m_pathWatched);
    this->m_watch.addPaths(this->getFileList(this->m_pathWatched, true));
    this->updateTreeView();
}
