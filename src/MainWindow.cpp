#include <QDebug>
#include "MainWindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_network(new NetworkManager(this)),
    m_watch(this),
    m_creation(this),
    m_open(this),
    m_connect(this->m_network),
    m_ignoredAdder(this),
    m_merger(this),
    m_project(NULL),
    m_user(NULL),
    m_stream(NULL),
    m_wsConnected(false)
{
    this->m_ui->setupUi(this);
    this->m_ui->addButton->setEnabled(false);
    this->m_ui->treeFile->setItemDelegate(new CustomStyle);
    this->m_ui->PullRequestWidget->hide();
    this->m_ui->PRList->setItemDelegate(new CustomPRItem);
    this->m_ignoredAdder.setTreeFile(this->m_ui->treeFile);
    this->m_network->moveToThread(&this->m_thread);
    qRegisterMetaType<QNetworkAccessManager::Operation>("QNetworkAccessManager::Operation");

    //BUTTONS CONNECT
    connect(this->m_ui->actionNewProject, SIGNAL(triggered()), &this->m_creation, SLOT(show()));
    connect(this->m_ui->actionOpenProject, &QAction::triggered, [=]() { this->m_open.init(this->m_user); });
    connect(this->m_ui->actionOpenProject, SIGNAL(triggered()), &this->m_open, SLOT(show()));
    connect(this->m_ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(this->m_ui->addButton, SIGNAL(pressed()), &this->m_ignoredAdder, SLOT(show()));
    connect(this->m_ui->startButton, SIGNAL(pressed()), this, SLOT(toggleConnection()));
    //FILE WATCHER CONNECT
    connect(&this->m_watch, SIGNAL(directoryChanged(QString)), this, SLOT(directoryWatchedChanged(QString)));
    connect(&this->m_watch, SIGNAL(fileChanged(QString)), this, SLOT(fileWatchedChanged(QString)));
    //DIALOG CONNECT
    connect(&this->m_creation, SIGNAL(accepted()), this, SLOT(projectCreated()));
    connect(&this->m_open, SIGNAL(accepted()), this, SLOT(projectOpen()));
    connect(&this->m_ignoredAdder, SIGNAL(accepted()), this, SLOT(addIgnored()));
    connect(&this->m_merger, SIGNAL(accepted()), this, SLOT(mergeFiles()));
    //UI CONNECT
    connect(this->m_ui->treeFile, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(addPathIgnored(QTreeWidgetItem *, int)));
    connect(this->m_ui->listIgnored->model(), SIGNAL(rowsInserted(QModelIndex, int, int)), this, SLOT(checkTreeIgnored()));
    connect(this->m_ui->listIgnored->model(), SIGNAL(rowsRemoved(QModelIndex, int, int)), this, SLOT(checkTreeIgnored()));
    connect((CustomPRItem *)this->m_ui->PRList->itemDelegate(), SIGNAL(mergeDelete(QModelIndex)), this, SLOT(removeMerge(QModelIndex)));
    connect((CustomPRItem *)this->m_ui->PRList->itemDelegate(), SIGNAL(mergeRequest(QModelIndex)), this, SLOT(prepareMerge(QModelIndex)));
    //NETWORK CONNECT
    connect(&this->m_connect, SIGNAL(request(QNetworkAccessManager::Operation,QString,QStringList,QStringList)),
            this, SIGNAL(request(QNetworkAccessManager::Operation,QString,QStringList,QStringList)));
    connect(&this->m_thread, SIGNAL(started()), this->m_network, SLOT(init()));
    connect(&this->m_thread, SIGNAL(started()), this, SLOT(on_actionDisconnect_triggered()));
    connect(&this->m_thread, SIGNAL(finished()), this->m_network, SLOT(deleteLater()));
    this->m_thread.start();
    connect(this->m_network, SIGNAL(responseReady(QNetworkReply*)), this, SLOT(networkResponse(QNetworkReply*)));
    connect(this->m_network, SIGNAL(filePartSent(QString,int,int)), this, SLOT(setProgressFile(QString,int,int)));
    connect(this->m_network, SIGNAL(wsConnection(bool)), this, SLOT(wsConnection(bool)));
    connect(this->m_network, SIGNAL(wsError(QAbstractSocket::SocketError,QString)), this, SLOT(wsError(QAbstractSocket::SocketError, QString)));
    connect(this->m_network, SIGNAL(requestError(NetworkResponse::NetworkError,QString)),
            this, SLOT(requestFailed(NetworkResponse::NetworkError,QString)));
    connect(this->m_network, SIGNAL(wsAuthenticated(QString)), this, SLOT(wsAuthenticated(QString)));
    connect(this->m_network, SIGNAL(wsFileSent(QString)), this, SLOT(fileSent(QString)));
    connect(this->m_network, SIGNAL(wsFileDeleted(QString)), this, SLOT(fileDeleted(QString)));
    connect(this->m_network, SIGNAL(newPullRequest(QJsonDocument)), this, SLOT(generatePullRequest(QJsonDocument)));
    connect(this->m_network, SIGNAL(wsFailAuth(QString)), this, SLOT(wsAuthenticatedFailed(QString)));
    connect(this->m_network, SIGNAL(wsFailWrite(QString, QString)), this, SLOT(fileError(QString, QString)));
    connect(this->m_network, SIGNAL(prFileReceived(QString, QString)), &this->m_merger, SLOT(prepareFile(QString,QString)));
    connect(this->m_network, SIGNAL(prFinished()), &this->m_merger, SLOT(show()));
}

MainWindow::~MainWindow(void)
{
    if (this->m_project != NULL)
        delete (this->m_project);
    if (this->m_user != NULL)
        delete (this->m_user);
    if (this->m_stream != NULL)
        delete (this->m_stream);
    this->m_thread.exit();
    delete (this->m_ui);
}

void MainWindow::loginResult(const QJsonDocument &document)
{
    QStringList headers;
    QJsonObject object = document.object();

    if (object.isEmpty()) {
        this->m_connect.failConnect("Unknown error");
        return;
    }
    if (!object["code"].isUndefined() && !object["code"].isNull()) {
        this->m_connect.failConnect(object["description"].toString());
        return;
    }
    this->m_user = new User(object["token"].toString());
    headers.append("Authorization=" + this->m_user->getToken());
    emit request(QNetworkAccessManager::GetOperation, "/users/" + this->m_user->getID(), QStringList(), headers);
}

void MainWindow::fetchUserResult(const QJsonDocument &document)
{
    QJsonObject object = document.object();

    if (object.isEmpty()) {
        this->m_connect.failConnect("Unknown error");
        return;
    }
    if (!object["code"].isUndefined() && !object["code"].isNull()) {
        this->m_connect.failConnect(object["description"].toString());
        return;
    }
    this->m_user->setUsername(object["name"].toString());
    this->m_connect.accept();
}

void MainWindow::fetchStreamsResult(const QJsonDocument &document)
{
    QList<Stream *> list;
    StreamsDialog stDialog;
    QJsonArray array;

    if (!document.object()["code"].isUndefined() && !document.object()["code"].isNull()) {
        QMessageBox::critical(this, "Error", "An error has occured :\n" + document.object()["description"].toString(), QMessageBox::Ok);
        return;
    }
    array = document.array();
    for (int i = 0; i < array.size(); i++) {
        Stream *stream = new Stream();

        stream->setId(array[i].toObject()["id"].toString());
        stream->setName(array[i].toObject()["title"].toString());
        list.append(stream);
    }
    if (list.count() == 0) {
        QMessageBox::critical(this, "No streams", "There is no stream to connect to", QMessageBox::Ok);
        this->m_ui->startButton->setText("Start streaming files");
        this->m_ui->startButton->setDisabled(false);
        return;
    }
    stDialog.setStreamList(list);
    if (stDialog.exec() == QDialog::Rejected) {
        this->m_ui->startButton->setDisabled(false);
        this->m_ui->startButton->setText("Start streaming files");
        return;
    }
    this->m_stream = stDialog.getSelected();
    emit request(QNetworkAccessManager::GetOperation, "/streams/" + this->m_stream->getId() + "/repository",
                 QStringList(), QStringList("Authorization=" + this->m_user->getToken()));
}

void MainWindow::fetchWebsocketResult(const QJsonDocument &document)
{
    QJsonObject object = document.object();

    if (object.isEmpty()) {
        QMessageBox::critical(this, "Error", "An unknown error has occured", QMessageBox::Ok);
        return;
    }
    if (!object["code"].isUndefined() && !object["code"].isNull()) {
        QMessageBox::critical(this, "Error", "An error has occured :\n" + object["description"].toString(), QMessageBox::Ok);
        return;
    }
    if (this->m_stream == NULL)
        return;
    this->m_stream->setWsToken(object["token"].toString());
    this->m_stream->setWsUrl(object["url"].toString());
    emit toggleWs(true, this->m_stream->getWsUrl());
}

void MainWindow::networkResponse(QNetworkReply *reply)
{
    static LinkFunc links[LINK_NUMBER] = {
        {"/login", &MainWindow::loginResult},
        {"/users/[\\d]+", &MainWindow::fetchUserResult},
        {"/streams/[\\d]+/repository", &MainWindow::fetchWebsocketResult},
        {"/users/[\\d]+/streams", &MainWindow::fetchStreamsResult}
    };
    QJsonDocument document;

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "ERROR : " << reply->errorString();
        if (!this->m_connect.isHidden()) {
            this->m_connect.failConnect(reply->errorString());
            return;
        }
        if (!this->m_ui->startButton->isEnabled()) {
            this->m_ui->startButton->setEnabled(true);
            this->m_ui->startButton->setText("Start streaming files");
        }
    }
    QByteArray response = reply->readAll();
    document = QJsonDocument::fromJson(response);
    for (int i = 0; i < LINK_NUMBER; i++) {
        QRegExp regexp(".*" + links[i].link);
        if (regexp.exactMatch(reply->request().url().toString())) {
            (this->*links[i].func)(document);
            break;
        }
    }
    reply->deleteLater();
}

void MainWindow::setProgressFile(const QString &name, int part, int maxPart)
{
    QList<QTreeWidgetItem *> list = this->findItems(this->m_ui->treeFile->topLevelItem(0), COLUMN_PATH, name);

    if (list.isEmpty())
        return;
    for (auto item : list) {
        item->setData(COLUMN_PROGRESS, Qt::DisplayRole, static_cast<float>(part) / static_cast<float>(maxPart) * 100.0);
        this->m_ui->treeFile->repaint();
    }
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

void MainWindow::deleteMerge(int row)
{
    QJsonDocument doc;
    QJsonObject data, object;

    data.insert("id", this->m_ui->PRList->item(row)->data(Qt::UserRole).toString());
    object.insert("data", data);
    object.insert("type", "pullrequest");
    object.insert("subtype", "delete");
    doc.setObject(object);
    emit sendText(QString::fromUtf8(doc.toJson()));
    delete this->m_ui->PRList->item(row);
}

void MainWindow::initProject(void)
{
    this->m_ui->addButton->setEnabled(this->m_project != NULL && this->m_project->getPath() != "");
    this->m_ui->titleLabel->setText((this->m_project == NULL ? "Select a project" : this->m_project->getTitle()));
    this->m_ui->watchedDirectory->setText((this->m_project == NULL ? "None" : this->m_project->getPath()));
    this->m_ui->actionPull_Request->setEnabled(this->m_project != NULL);
    this->m_ui->actionPull_Request->setChecked(false);
    this->m_merger.setProjectPath(this->m_project == NULL ? NULL : this->m_project->getPath());
}

void MainWindow::initWatcher(void)
{
    this->m_watch.addPath(this->m_project->getPath());
    this->m_watch.addPaths(this->getFileList(this->m_project->getPath(), true));
    this->m_ui->treeFile->addTopLevelItem(this->getTreeItem(this->m_project->getPath()));
    this->m_ui->treeFile->setRootIndex(this->m_ui->treeFile->model()->index(0, COLUMN_NAME));
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
    QFile conf(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/" + this->m_user->getUsername() + "/.oldPath"));
    QTextStream stream(&conf);
    QStringList olds;
    QString line;

    if (!QDir(QCoreApplication::applicationDirPath() + "/" + this->m_user->getUsername()).exists())
        QDir().mkpath(QCoreApplication::applicationDirPath() + "/" + this->m_user->getUsername());
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

    item->setData(COLUMN_PATH, Qt::DisplayRole, dir.path());
    item->setData(COLUMN_PROGRESS, Qt::DisplayRole, -3);
    for (auto file : this->m_watch.files()) {
        QFileInfo f(file);

        if (f.absoluteDir() == dir.absolutePath()) {
            QTreeWidgetItem *tmp = new QTreeWidgetItem(QStringList(f.fileName()));
            tmp->setData(COLUMN_CHECKSTATE, Qt::CheckStateRole, Qt::Checked);
            tmp->setData(COLUMN_PROGRESS, Qt::DisplayRole, 0);
            tmp->setData(COLUMN_PATH, Qt::DisplayRole, f.filePath());
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
            if (item->child(i)->data(COLUMN_PATH, Qt::DisplayRole).toString() == str)
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
            if (item->child(i)->data(COLUMN_PATH, Qt::DisplayRole).toString() == str)
                check = true;
        if (!check)
            result.append(item->child(i));
    }
    return (result);
}

QList<QTreeWidgetItem *> MainWindow::findItems(QTreeWidgetItem *item, int column, const QString &check) const
{
    QList<QTreeWidgetItem *> list;
    QRegExp regexp(QDir::cleanPath("*/" + this->m_ui->treeFile->topLevelItem(0)->text(COLUMN_NAME) + "/" + check));

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
    bool result = regexp.exactMatch(item->data(COLUMN_PATH, Qt::DisplayRole).toString());

    if (!result && item->parent() != NULL)
        result = this->matchIgnoredRec(item->parent(), regexp);
    return (result);
}

int MainWindow::connectUser(void)
{
    return (this->m_connect.exec());
}

void MainWindow::wsAuthenticated(const QString &message)
{
    qDebug() << "Successfully authentified on server with message " << message;
}

void MainWindow::fileSent(const QString &filename)
{
    QList<QTreeWidgetItem *> list = this->findItems(this->m_ui->treeFile->topLevelItem(0), COLUMN_PATH, filename);

    if (list.isEmpty())
        return;
    for (auto item : list) {
        item->setData(COLUMN_PROGRESS, Qt::DisplayRole, 101);
        this->m_ui->treeFile->repaint();
    }
}

void MainWindow::fileDeleted(const QString &filename)
{
    qDebug() << "File " << filename << " successfully deleted from server";
}

void MainWindow::generatePullRequest(const QJsonDocument &doc)
{
    QListWidgetItem *item = new QListWidgetItem();
    QJsonObject data = doc.object();
    QStringList files;

    for (auto file : data["files"].toArray())
        files << file.toString();
    item->setData(Qt::DisplayRole, data["title"].toString());
    item->setData(Qt::UserRole, data["id"].toString());
    item->setData(Qt::UserRole + 1, data["owner"].toString());
    item->setData(Qt::UserRole + 2, data["description"].toString());
    item->setData(Qt::UserRole + 3, QDateTime::fromMSecsSinceEpoch(data["date"].toVariant().toLongLong()));
    item->setData(Qt::UserRole + 4, files);
    this->m_ui->PRList->insertItem(0, item);
}

void MainWindow::wsAuthenticatedFailed(const QString &message)
{
    qDebug() << "Authentication failed with message " << message;
    this->toggleConnection();
}

void MainWindow::fileError(const QString &filename, const QString &error)
{
    QList<QTreeWidgetItem *> list = this->findItems(this->m_ui->treeFile->topLevelItem(0), COLUMN_PATH, filename);

    qDebug() << "File " << filename << " returned an error with message " << error;
    if (list.isEmpty())
        return;
    for (auto item : list) {
        item->setData(COLUMN_PROGRESS, Qt::DisplayRole, -2);
        this->m_ui->treeFile->repaint();
    }
}

void MainWindow::removeMerge(const QModelIndex &index)
{
    this->deleteMerge(index.row());
}

void MainWindow::prepareMerge(const QModelIndex &index)
{
    QJsonDocument document;
    QJsonObject data, pr;

    data.insert("id", index.data(Qt::UserRole).toString());
    pr.insert("type", "pullrequest");
    pr.insert("subtype", "get");
    pr.insert("data", data);
    document.setObject(pr);
    emit sendText(QString::fromUtf8(document.toJson()));
    this->m_merger.clean();
    this->m_merger.init(index.data(Qt::UserRole).toString());
}

void MainWindow::wsConnection(bool connected)
{
    this->m_wsConnected = connected;
    if (this->m_wsConnected) {
        this->m_ui->startButton->setText("Stop streaming files");
        this->m_ui->startButton->setDisabled(false);
        this->authenticate();
    }
    else {
        this->m_ui->PRList->clear();
        this->resetSync(this->m_ui->treeFile->topLevelItem(0));
    }
}

void MainWindow::wsError(QAbstractSocket::SocketError error, const QString &errorString)
{
    qDebug() << error;
    QMessageBox::critical(this, "Websocket error", "Cannot connect to websocket :\n" + errorString, QMessageBox::Ok);
    this->m_ui->startButton->setDisabled(false);
    this->m_ui->startButton->setText("Start streaming files");
}

void MainWindow::toggleConnection(void)
{
    if (this->m_ui->startButton->text().contains("Start")) {
        this->m_ui->startButton->setDisabled(true);
        this->m_ui->startButton->setText("Connecting...");
        emit request(QNetworkAccessManager::GetOperation, "/users/" + this->m_user->getID() + "/streams",
                     QStringList(), QStringList("Authorization=" + this->m_user->getToken()));
    }
    else if (this->m_ui->startButton->text().contains("Stop")) {
        this->m_ui->startButton->setText("Start streaming files");
        emit toggleWs(false, "");
    }
}

void MainWindow::requestFailed(NetworkResponse::NetworkError error, const QString &msg)
{
    qDebug() << error;
    qDebug() << msg;
    if (error == NetworkResponse::NETWORK_NOT_CONNECTED) {
        if (!this->m_connect.isHidden())
            this->m_connect.tryReconnect();
    }
}

void MainWindow::checkPath(QTreeWidgetItem *item, const QString &path, const QStringList &list)
{
    if (item->data(COLUMN_PATH, Qt::DisplayRole).toString() == path) {
        QList<QTreeWidgetItem *> removes = this->getRemovedPaths(item, list);
        QList<QString> adds = this->getAddedPaths(item, list);

        if (adds.size() == 1 && removes.size() == 1) {
            QFileInfo fi(adds.at(0));

            if (!removes.at(0)->font(COLUMN_NAME).strikeOut())
                emit sendRemoveFile(removes.at(0)->text(COLUMN_PATH).replace(QDir::cleanPath(this->m_ui->treeFile->topLevelItem(0)->text(COLUMN_PATH) + "/"), ""));
            removes.at(0)->setData(COLUMN_NAME, Qt::DisplayRole, fi.fileName());
            removes.at(0)->setData(COLUMN_PATH, Qt::DisplayRole, adds.at(0));
            QFile *file = new QFile(adds.at(0));
            if (!removes.at(0)->font(COLUMN_NAME).strikeOut())
                emit sendWriteFile(file, file->fileName().replace(QDir::cleanPath(this->m_ui->treeFile->topLevelItem(0)->text(COLUMN_PATH) + "/"), ""));
            return;
        }
        for (auto add : adds) {
            QFileInfo fi(add);
            QTreeWidgetItem *newItem = new QTreeWidgetItem(QStringList(fi.fileName()));

            if (fi.isFile())
                newItem->setData(COLUMN_CHECKSTATE, Qt::CheckStateRole, Qt::Checked);
            newItem->setData(COLUMN_PATH, Qt::DisplayRole, add);
            item->addChild(newItem);
            this->checkTreeIgnored(false);
            QFile *file = new QFile(add);
            if (!newItem->font(COLUMN_NAME).strikeOut())
                emit sendWriteFile(file, file->fileName().replace(QDir::cleanPath(this->m_ui->treeFile->topLevelItem(0)->text(COLUMN_PATH) + "/"), ""));
        }
        for (auto remove : removes) {
            if (!remove->font(COLUMN_NAME).strikeOut())
                emit sendRemoveFile(remove->text(COLUMN_PATH).replace(QDir::cleanPath(this->m_ui->treeFile->topLevelItem(0)->text(COLUMN_PATH) + "/"), ""));
            delete remove;
        }
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
    QList<QTreeWidgetItem *> list;
    QFile *file = new QFile(path);

    if (!file->exists())
        this->m_watch.removePath(path);
    else if (this->m_wsConnected) {
        list = this->findItems(this->m_ui->treeFile->topLevelItem(0), COLUMN_PATH, path);
        QApplication::processEvents();
        for (auto item : list)
            if (!item->font(COLUMN_NAME).strikeOut())
                emit sendWriteFile(file, file->fileName().replace(QDir::cleanPath(this->m_ui->treeFile->topLevelItem(0)->text(COLUMN_PATH) + "/"), ""));
    }
}

void MainWindow::addPathIgnored(QTreeWidgetItem *item, int column)
{
    QRegExp regexp;

    if (item == NULL || column != COLUMN_CHECKSTATE)
        return;
    regexp.setPatternSyntax(QRegExp::Wildcard);
    regexp.setCaseSensitivity(Qt::CaseSensitive);
    if (item->data(column, Qt::CheckStateRole) == Qt::Unchecked) {
        QListWidgetItem *newItem = new QListWidgetItem(item->data(COLUMN_PATH, Qt::DisplayRole).toString());

        newItem->setText(newItem->text().replace(QDir::cleanPath(this->m_ui->treeFile->topLevelItem(0)->data(COLUMN_PATH, Qt::DisplayRole).toString() + "/"), ""));
        newItem->setData(Qt::UserRole, item->data(COLUMN_PATH, Qt::DisplayRole));
        this->m_ui->listIgnored->addItem(newItem);
    }
    else if (item->data(column, Qt::CheckStateRole) == Qt::Checked)
        for (int i = 0; i < this->m_ui->listIgnored->count();) {
            regexp.setPattern("*/" + this->m_ui->listIgnored->item(i)->text());
            if (this->m_ui->listIgnored->item(i)->data(Qt::UserRole).isValid() &&
                    this->m_ui->listIgnored->item(i)->data(Qt::UserRole).toString() == item->data(COLUMN_PATH, Qt::DisplayRole).toString())
                delete (this->m_ui->listIgnored->item(i));
            else if ((!this->m_ui->listIgnored->item(i)->data(Qt::UserRole).isValid() ||
                      this->m_ui->listIgnored->item(i)->data(Qt::UserRole).toString() == "") &&
                     this->matchIgnoredRec(item, regexp)) {
                if (QMessageBox::warning(this, "Remove path matching", "There is a conflict for reactivating the item with : " +
                                         this->m_ui->listIgnored->item(i)->text() + "\nDelete?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
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
    QFont font(item->font(COLUMN_NAME));

    font.setStrikeOut(false);
    item->setFont(COLUMN_NAME, font);
    if (item->data(COLUMN_PROGRESS, Qt::DisplayRole) == -1)
        item->setData(COLUMN_PROGRESS, Qt::DisplayRole, 0);
    if (item->data(COLUMN_CHECKSTATE, Qt::CheckStateRole) == Qt::Unchecked)
        item->setData(COLUMN_CHECKSTATE, Qt::CheckStateRole, Qt::Checked);
    for (int i = 0; i < item->childCount(); i++)
        this->resetFont(item->child(i));
}

void MainWindow::applyFont(QTreeWidgetItem *item)
{
    QFont font(item->font(COLUMN_NAME));
    bool allIgnored = true;

    for (int i = 0; i < item->childCount(); i++)
    {
        QFont fontChild(item->child(i)->font(COLUMN_NAME));

        fontChild.setStrikeOut(true);
        if (font.strikeOut()) {
            item->child(i)->setFont(COLUMN_NAME, fontChild);
            if (item->child(i)->data(COLUMN_PROGRESS, Qt::DisplayRole) != -3)
                item->child(i)->setData(COLUMN_PROGRESS, Qt::DisplayRole, -1);
            if (item->child(i)->data(COLUMN_CHECKSTATE, Qt::CheckStateRole) == Qt::Checked)
                item->child(i)->setData(COLUMN_CHECKSTATE, Qt::CheckStateRole, Qt::Unchecked);
        }
        else if (item->child(i)->data(COLUMN_PROGRESS, Qt::DisplayRole) == 0)
            this->fileWatchedChanged(item->child(i)->data(COLUMN_PATH, Qt::DisplayRole).toString());
        allIgnored = allIgnored && item->child(i)->font(COLUMN_NAME).strikeOut();
        if (item->child(i)->childCount() > 0)
            this->applyFont(item->child(i));
    }
    font.setStrikeOut(allIgnored);
    item->setFont(COLUMN_NAME, font);
    if (allIgnored)
        this->m_ui->treeFile->collapseItem(item);
    else
        this->m_ui->treeFile->expandItem(item);
}

void MainWindow::resetSync(QTreeWidgetItem *elem)
{
    if (elem->data(COLUMN_PROGRESS, Qt::DisplayRole).toInt() >= 0)
        elem->setData(COLUMN_PROGRESS, Qt::DisplayRole, 0);
    if (elem->childCount() > 0)
        for (int i = 0; i < elem->childCount(); i++)
            this->resetSync(elem->child(i));
}

void MainWindow::uncheckedItemsRec(QTreeWidgetItem *elem)
{
    if (elem->data(COLUMN_PROGRESS, Qt::DisplayRole) != -3)
        elem->setData(COLUMN_PROGRESS, Qt::DisplayRole, -1);
    if (elem->data(COLUMN_CHECKSTATE, Qt::CheckStateRole) == Qt::Checked)
        elem->setData(COLUMN_CHECKSTATE, Qt::CheckStateRole, Qt::Unchecked);
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
        list = this->findItems(this->m_ui->treeFile->topLevelItem(0), COLUMN_PATH,
                               (listItem->data(Qt::UserRole).isValid() && listItem->data(Qt::UserRole).toString() != ""
                ? listItem->data(Qt::UserRole).toString() : listItem->text()));
        for (auto elem : list) {
            QFont font(elem->font(COLUMN_NAME));

            font.setStrikeOut(true);
            elem->setFont(COLUMN_NAME, font);
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

    if (this->m_wsConnected)
        this->toggleConnection();
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
    this->m_ui->listIgnored->clear();
    this->initProject();
    this->initWatcher();
    this->initIgnored();
    this->saveLastProject();
    this->m_ui->startButton->setEnabled(true);
}

void MainWindow::projectOpen(void)
{
    Project *project = this->m_open.getProject();

    if (this->m_wsConnected)
        this->toggleConnection();
    if (project == NULL)
        return;
    if (this->m_project != NULL)
        delete (this->m_project);
    this->m_project = project;
    this->clearWatcher();
    this->m_ui->listIgnored->clear();
    this->initProject();
    this->initWatcher();
    this->initIgnored();
    this->saveLastProject();
    this->m_ui->startButton->setEnabled(true);
    this->m_ui->actionPull_Request->setChecked(true);
}

void MainWindow::addIgnored(void)
{
    this->m_ui->listIgnored->addItem(this->m_ignoredAdder.getPath());
}

void MainWindow::mergeFiles(void)
{
    QMap<QString, QStringList> files = this->m_merger.getMergingContent();

    for (auto filename : files.keys()) {
        QFile file(QDir::cleanPath(this->m_project->getPath() + filename));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
            continue;
        file.write(QString(files.value(filename).join("\r\n")).toUtf8());
        file.close();
    }
    for (int i = 0; i < this->m_ui->PRList->count(); i++)
        if (this->m_ui->PRList->item(i)->data(Qt::UserRole).toString() == this->m_merger.getID()) {
            this->deleteMerge(i);
            break;
        }
}

void MainWindow::authenticate(void)
{
    QJsonDocument document;
    QJsonObject object, data, file;

    object.insert("type", "authenticate");
    file.insert("username", this->m_user->getUsername());
    file.insert("project", this->m_stream->getName());
    data.insert("file", file);
    data.insert("token", this->m_stream->getWsToken());
    object.insert("data", data);
    document.setObject(object);
    emit sendText(QString::fromUtf8(document.toJson()));
    for (auto file : this->m_watch.files())
        this->fileWatchedChanged(file);
}

void MainWindow::on_removeButton_pressed(void)
{
    for (auto item : this->m_ui->listIgnored->selectedItems())
        delete (item);
}

void MainWindow::on_actionDisconnect_triggered(void)
{
    this->hide();
    if (this->m_wsConnected)
        this->toggleConnection();
    if (this->connectUser() == QDialog::Rejected) {
        this->close();
        return;
    }
    this->clearWatcher();
    this->m_ui->listIgnored->clear();
    if (this->m_project != NULL)
        delete (this->m_project);
    if (this->m_stream != NULL)
        delete (this->m_stream);
    this->m_project = NULL;
    this->m_stream = NULL;
    this->initProject();
    this->m_ui->startButton->setEnabled(false);
    this->show();
}

void MainWindow::on_actionPull_Request_changed(void)
{
    if (this->m_ui->actionPull_Request->isChecked())
        this->m_ui->PullRequestWidget->show();
    else
        this->m_ui->PullRequestWidget->hide();
}
