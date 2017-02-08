#include <QDebug>
#include "MainWindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_network(new NetworkManager),
    m_creation(this),
    m_open(this),
    m_connect(this->m_network),
    m_ignoredAdder(this),
    m_project(NULL)
{
    this->m_ui->setupUi(this);
    this->m_ui->addButton->setEnabled(false);
    this->m_ignoredAdder.setTreeFile(this->m_ui->treeFile);
    connect(this->m_ui->actionNewProject, SIGNAL(triggered()), &this->m_creation, SLOT(show()));
    connect(this->m_ui->actionOpenProject, SIGNAL(triggered()), &this->m_open, SLOT(init()));
    connect(this->m_ui->actionOpenProject, SIGNAL(triggered()), &this->m_open, SLOT(show()));
    connect(this->m_ui->addButton, SIGNAL(pressed()), &this->m_ignoredAdder, SLOT(show()));
    connect(&this->m_creation, SIGNAL(accepted()), this, SLOT(projectCreated()));
    connect(&this->m_open, SIGNAL(accepted()), this, SLOT(projectOpen()));
    connect(&this->m_ignoredAdder, SIGNAL(accepted()), this, SLOT(addIgnored()));
    connect(&this->m_watch, SIGNAL(directoryChanged(QString)), this, SLOT(directoryWatchedChanged(QString)));
    connect(&this->m_watch, SIGNAL(fileChanged(QString)), this, SLOT(fileWatchedChanged(QString)));
    connect(this->m_ui->treeFile, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(addPathIgnored(QTreeWidgetItem *, int)));
    connect(this->m_ui->listIgnored->model(), SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(checkTreeIgnored()));
    connect(this->m_ui->listIgnored->model(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(checkTreeIgnored()));
}

MainWindow::~MainWindow(void)
{
    if (this->m_project != NULL)
        delete (this->m_project);
    delete (this->m_network);
    delete (this->m_ui);
}

void MainWindow::clearWatcher(void)
{
    QStringList listNotRemoved;

    if (!this->m_watch.files().empty())
        listNotRemoved = this->m_watch.removePaths(this->m_watch.files());
    if (!this->m_watch.directories().empty())
        listNotRemoved.append(this->m_watch.removePaths(this->m_watch.directories()));
    if (listNotRemoved.size() != 0)
        listNotRemoved = this->m_watch.removePaths(listNotRemoved);
    this->m_ui->treeFile->clear();
}

void MainWindow::clearIgnored(void)
{
    disconnect(this->m_ui->listIgnored->model(), SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(checkTreeIgnored()));
    disconnect(this->m_ui->listIgnored->model(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(checkTreeIgnored()));
    this->m_ui->listIgnored->clear();
    connect(this->m_ui->listIgnored->model(), SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(checkTreeIgnored()));
    connect(this->m_ui->listIgnored->model(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(checkTreeIgnored()));
}

void MainWindow::initProject(void)
{
    this->m_ui->addButton->setEnabled(this->m_project != NULL && this->m_project->getPath() != "");
    this->m_ui->titleLabel->setText((this->m_project == NULL ? "Select a project" : this->m_project->getTitle()));
    this->m_ui->watchedDirectory->setText((this->m_project == NULL ? "None" : this->m_project->getPath()));
}

void MainWindow::initWatcher(void)
{
    this->m_watch.addPath(this->m_project->getPath());
    this->m_watch.addPaths(this->getFileList(this->m_project->getPath(), true));
    this->m_ui->treeFile->addTopLevelItem(this->getTreeItem(this->m_project->getPath()));
    this->m_ui->treeFile->setRootIndex(this->m_ui->treeFile->model()->index(0, 0));
    this->m_ui->treeFile->expandAll();
}

void MainWindow::initIgnored(void)
{
    QList<QString> ignoredList = this->m_project->retrieveIgnored();

    disconnect(this->m_ui->listIgnored->model(), SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(checkTreeIgnored()));
    disconnect(this->m_ui->listIgnored->model(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(checkTreeIgnored()));
    for (auto ignore : ignoredList)
        this->m_ui->listIgnored->addItem(ignore);
    this->checkTreeIgnored(false);
    connect(this->m_ui->listIgnored->model(), SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(checkTreeIgnored()));
    connect(this->m_ui->listIgnored->model(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(checkTreeIgnored()));
}

void MainWindow::saveLastProject(void)
{
    QFile conf(QCoreApplication::applicationDirPath() + "/.oldPath");
    QTextStream stream(&conf);
    QStringList olds;
    QString line;

    if (!conf.open(QFile::ReadWrite))
        return;
    while (!stream.atEnd()) {
        line = stream.readLine();
        if (line.trimmed() != "" && line != QDir::cleanPath(this->m_project->getPath() + "/.tcconf"))
            olds.append(line);
    }
    olds.push_front(QDir::cleanPath(this->m_project->getPath() + "/.tcconf"));
    conf.resize(0);
    for (auto old : olds)
        stream << old << "\n";
    conf.close();
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

QList<QTreeWidgetItem *> MainWindow::findItems(QTreeWidgetItem *item, int column, const QString &check) const
{
    QList<QTreeWidgetItem *> list;
    QRegExp regexp("*/" + this->m_ui->treeFile->topLevelItem(0)->text(0) + "/" + check);

    regexp.setPatternSyntax(QRegExp::Wildcard);
    regexp.setCaseSensitivity(Qt::CaseSensitive);
    if (item != this->m_ui->treeFile->topLevelItem(0) &&
            (regexp.exactMatch(item->text(column)) || check == item->text(column)))
        list.append(item);
    for (int i = 0; i < item->childCount(); i++)
        list.append(this->findItems(item->child(i), column, check));
    return (list);
}

bool MainWindow::matchIgnoredRec(QTreeWidgetItem *item, const QRegExp &regexp) const
{
    bool result = regexp.exactMatch(item->data(2, Qt::DisplayRole).toString());

    if (!result && item->parent() != NULL)
        result = this->matchIgnoredRec(item->parent(), regexp);
    return (result);
}

int MainWindow::connectUser(void)
{
    return (this->m_connect.exec());
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
            regexp.setPattern("*/" + this->m_ui->listIgnored->item(i)->text());
            if (this->m_ui->listIgnored->item(i)->data(Qt::UserRole).isValid() &&
                    this->m_ui->listIgnored->item(i)->data(Qt::UserRole).toString() == item->data(2, Qt::DisplayRole).toString())
                delete (this->m_ui->listIgnored->item(i));
            else if ((!this->m_ui->listIgnored->item(i)->data(Qt::UserRole).isValid() ||
                      this->m_ui->listIgnored->item(i)->data(Qt::UserRole).toString() == "") &&
                     this->matchIgnoredRec(item, regexp)) {
                if (QMessageBox::warning(this, "Remove path matching",
                                         "There is a conflict for reactivating the item with : " +
                                         this->m_ui->listIgnored->item(i)->text() + "\nDelete?",
                                         QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
                    delete (this->m_ui->listIgnored->item(i));
                else {
                    this->applyFont(this->m_ui->treeFile->topLevelItem(0));
                    return;
                }
            }
            else
                i++;
        }
}

void MainWindow::resetFont(QTreeWidgetItem *item)
{
    QFont font(item->font(0));

    font.setStrikeOut(false);
    item->setFont(0, font);
    if (item->data(1, Qt::CheckStateRole) == Qt::Unchecked)
        item->setData(1, Qt::CheckStateRole, Qt::Checked);
    for (int i = 0; i < item->childCount(); i++)
        this->resetFont(item->child(i));
}

void MainWindow::applyFont(QTreeWidgetItem *item)
{
    QFont font(item->font(0));
    bool allIgnored = true;

    for (int i = 0; i < item->childCount(); i++)
    {
        QFont fontChild(item->child(i)->font(0));

        fontChild.setStrikeOut(true);
        if (font.strikeOut()) {
            item->child(i)->setFont(0, fontChild);
            if (item->child(i)->data(1, Qt::CheckStateRole) == Qt::Checked)
                item->child(i)->setData(1, Qt::CheckStateRole, Qt::Unchecked);
        }
        allIgnored = allIgnored && item->child(i)->font(0).strikeOut();
        if (item->child(i)->childCount() > 0)
            this->applyFont(item->child(i));
    }
    font.setStrikeOut(allIgnored);
    item->setFont(0, font);
    if (allIgnored)
        this->m_ui->treeFile->collapseItem(item);
    else
        this->m_ui->treeFile->expandItem(item);
}

void MainWindow::uncheckedItemsRec(QTreeWidgetItem *elem)
{
    if (elem->data(1, Qt::CheckStateRole) == Qt::Checked)
        elem->setData(1, Qt::CheckStateRole, Qt::Unchecked);
    for (int i = 0; i < elem->childCount(); i++)
        this->uncheckedItemsRec(elem->child(i));
}

void MainWindow::checkTreeIgnored(bool save)
{
    QList<QTreeWidgetItem *> list;
    QListWidgetItem *listItem;
    QList<QString> ignoreList;

    this->resetFont(this->m_ui->treeFile->topLevelItem(0));
    for (int i = 0; i < this->m_ui->listIgnored->count(); i++) {
        listItem = this->m_ui->listIgnored->item(i);
        list = this->findItems(this->m_ui->treeFile->topLevelItem(0), 2,
                               (listItem->data(Qt::UserRole).isValid() && listItem->data(Qt::UserRole).toString() != ""
                ? listItem->data(Qt::UserRole).toString() : listItem->text()));
        for (auto elem : list) {
            QFont font(elem->font(0));

            font.setStrikeOut(true);
            elem->setFont(0, font);
            this->uncheckedItemsRec(elem);
        }
        ignoreList.append(listItem->text());
    }
    this->applyFont(this->m_ui->treeFile->topLevelItem(0));
    if (save)
        this->m_project->saveIgnored(ignoreList);
}

void MainWindow::projectCreated(void)
{
    Project *project = this->m_creation.getProject();

    if (project->getPath() == "")
        return;
    if (!project->save()) {
        QMessageBox::critical(this, "Project error", "Error : Can't create project.", QMessageBox::Ok);
        return;
    }
    if (this->m_project != NULL)
        delete (this->m_project);
    this->m_project = project;
    this->clearWatcher();
    this->clearIgnored();
    this->initProject();
    this->initWatcher();
    this->initIgnored();
    this->saveLastProject();
}

void MainWindow::projectOpen(void)
{
    Project *project = this->m_open.getProject();

    if (project == NULL)
        return;
    if (this->m_project != NULL)
        delete (this->m_project);
    this->m_project = project;
    this->clearWatcher();
    this->clearIgnored();
    this->initProject();
    this->initWatcher();
    this->initIgnored();
    this->saveLastProject();
}

void MainWindow::addIgnored(void)
{
    this->m_ui->listIgnored->addItem(this->m_ignoredAdder.getPath());
}

void MainWindow::on_actionExit_triggered(void)
{
    this->close();
}

void MainWindow::on_removeButton_pressed(void)
{
    for (auto item : this->m_ui->listIgnored->selectedItems())
        delete (item);
}

void MainWindow::on_actionDisconnect_triggered(void)
{
    this->hide();
    if (this->connectUser() == QDialog::Rejected) {
        this->close();
        return;
    }
    this->clearWatcher();
    this->clearIgnored();
    if (this->m_project != NULL)
        delete (this->m_project);
    this->m_project = NULL;
    this->initProject();
    this->show();
}
