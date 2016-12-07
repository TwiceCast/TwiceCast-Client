#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QFileSystemWatcher>
#include <QTreeWidget>

#include "ProjectCreationDialog.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    QStringList getFileList(const QString &path, bool recursively = false) const;
    QTreeWidgetItem *getTreeItem(const QString &) const;
    QList<QTreeWidgetItem *> getRemovedPaths(QTreeWidgetItem *item, const QStringList &list) const;
    QList<QString> getAddedPaths(QTreeWidgetItem *item, const QStringList &list) const;

private:
    void removeByPath(QTreeWidgetItem *item, const QString &path);
    void checkPath(QTreeWidgetItem *item, const QString &path, const QStringList &list);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void directoryWatchedChanged(const QString &);
    void fileWatchedChanged(const QString &);
    void projectCreated(void);
    void on_actionExit_triggered();

private:
    Ui::MainWindow *m_ui;
    QFileSystemWatcher m_watch;
    ProjectCreationDialog m_creation;
    QString m_pathWatched;
};

#endif // MAINWINDOW_HPP
