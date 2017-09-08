#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QMessageBox>
#include <QRegExp>
#include <QThread>

#include "ConnectionWindow.hpp"
#include "ProjectCreationDialog.hpp"
#include "ProjectOpenDialog.hpp"
#include "IgnoredAdderDialog.hpp"
#include "StreamsDialog.hpp"
#include "CustomStyle.hpp"
#include "User.hpp"
#include "Stream.hpp"

#define LINK_NUMBER 4

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    struct LinkFunc {
        QString link;
        void (MainWindow::*func)(const QJsonDocument &);
    };

private:
    QStringList getFileList(const QString &, bool recursively = false) const;
    QTreeWidgetItem *getTreeItem(const QString &) const;
    QList<QTreeWidgetItem *> getRemovedPaths(QTreeWidgetItem *, const QStringList &) const;
    QList<QString> getAddedPaths(QTreeWidgetItem *, const QStringList &) const;
    QList<QTreeWidgetItem *> findItems(QTreeWidgetItem *, int, const QString &) const;
    bool matchIgnoredRec(QTreeWidgetItem *, const QRegExp &) const;

private:
    void loginResult(const QJsonDocument &);
    void fetchUserResult(const QJsonDocument &);
    void fetchWebsocketResult(const QJsonDocument &);
    void fetchStreamsResult(const QJsonDocument &);
    void authenticate(void);
    void clearWatcher(void);
    void clearIgnored(void);
    void initProject(void);
    void initWatcher(void);
    void initIgnored(void);
    void saveLastProject(void);
    void removeByPath(QTreeWidgetItem *, const QString &);
    void checkPath(QTreeWidgetItem *, const QString &, const QStringList &);
    void applyFont(QTreeWidgetItem *);
    void resetFont(QTreeWidgetItem *);
    void uncheckedItemsRec(QTreeWidgetItem *);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    int connectUser(void);

signals:
    void request(QNetworkAccessManager::Operation, const QString &, const QStringList &, const QStringList &headers = QStringList());
    void sendWriteFile(QFile *, const QString &);
    void sendRemoveFile(const QString &);
    void sendText(const QString &);
    void toggleWs(bool, const QString &);

private slots:
    void wsConnection(bool connected = true);
    void wsError(QAbstractSocket::SocketError, const QString &);
    void toggleConnection(void);
    void requestFailed(NetworkResponse::NetworkError, const QString &);
    void directoryWatchedChanged(const QString &);
    void fileWatchedChanged(const QString &);
    void addPathIgnored(QTreeWidgetItem *, int);
    void checkTreeIgnored(bool save = true);
    void networkResponse(QNetworkReply *);
    void setProgressFile(const QString &, int, int);
    void projectCreated(void);
    void projectOpen(void);
    void addIgnored(void);
    void on_removeButton_pressed(void);
    void on_actionDisconnect_triggered(void);
    void on_actionPull_Request_changed(void);

private:
    Ui::MainWindow *m_ui;
    NetworkManager *m_network;
    QThread m_thread;
    QFileSystemWatcher m_watch;
    ProjectCreationDialog m_creation;
    ProjectOpenDialog m_open;
    ConnectionWindow m_connect;
    IgnoredAdderDialog m_ignoredAdder;
    Project *m_project;
    User *m_user;
    Stream *m_stream;
    bool m_wsConnected;
};

#endif // MAINWINDOW_HPP
